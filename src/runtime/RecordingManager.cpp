#include <multigauge/runtime/RecordingManager.h>

#include <multigauge/io/Log.h>
#include <multigauge/io/FileSystem.h>

#include <etl/crc32.h>

#include <algorithm>
#include <bit>
#include <cmath>
#include <cstdio>
#include <limits>
#include <new>
#include <string>
#include <utility>

namespace mg {
namespace {

// Wire layout and recovery rules are specified in docs/recording-format.md.
constexpr std::size_t BlockBytes = 512;
constexpr std::size_t BlockHeaderBytes = 9;
constexpr std::size_t SampleBytes = 9;
constexpr std::size_t ChecksumBytes = 4;
constexpr std::size_t SamplesPerBlock = (BlockBytes - BlockHeaderBytes - ChecksumBytes) / SampleBytes;
constexpr auto MaxOffset = std::numeric_limits<std::uint32_t>::max();
constexpr auto MaxValue = std::numeric_limits<float>::max();
constexpr std::size_t MaxChannelIdBytes = 128;
constexpr std::size_t MaxUnitLabelBytes = 32;

static_assert(sizeof(float) == 4 && std::numeric_limits<float>::is_iec559);

std::string utcFilenameStem(std::chrono::sys_seconds startedAt) {
    using namespace std::chrono;
    if (startedAt < sys_days{year{1}/January/1} || startedAt >= sys_days{year{10000}/January/1}) return {};

    const auto day = floor<days>(startedAt);
    const year_month_day date{day};
    const hh_mm_ss time{startedAt - day};
    char stem[32];
    std::snprintf(stem, sizeof(stem), "%04d-%02u-%02u_%02d-%02d-%02d",
                  int(date.year()), unsigned(date.month()), unsigned(date.day()),
                  int(time.hours().count()), int(time.minutes().count()), int(time.seconds().count()));
    return stem;
}

std::string recordingPath(std::string_view utcStem, std::uint32_t number) {
    std::string path = "/recordings/";
    path += utcStem;
    if (utcStem.empty() || number != 0) {
        path += utcStem.empty() ? "recording_" : "_";
        const auto digits = std::to_string(number);
        const std::size_t width = utcStem.empty() ? 6 : 3;
        if (digits.size() < width) path.append(width - digits.size(), '0');
        path += digits;
    }
    path += ".mgr";
    return path;
}

template <typename UInt>
void putLittleEndian(std::uint8_t* out, UInt value) {
    for (std::size_t i = 0; i < sizeof(value); ++i) {
        out[i] = static_cast<std::uint8_t>(value);
        value >>= 8;
    }
}

bool validChannels(std::span<const RecordingManager::Channel> channels) {
    if (channels.empty() || channels.size() > std::numeric_limits<std::uint8_t>::max()) return false;

    for (std::size_t i = 0; i < channels.size(); ++i) {
        const auto& channel = channels[i];
        if (channel.id.empty() || channel.id.size() > MaxChannelIdBytes || channel.unit.size() > MaxUnitLabelBytes) {
            return false;
        }
        for (const auto& earlier : channels.first(i)) {
            if (channel.id == earlier.id) return false;
        }
    }
    return true;
}

bool writeHeader(io::File& file, std::span<const RecordingManager::Channel> channels,
                 std::optional<std::chrono::sys_seconds> startedAt) {
    etl::crc32_t16 crc;
    const auto write = [&](std::span<const std::uint8_t> bytes) {
        if (bytes.empty()) return true;
        crc.add(bytes.begin(), bytes.end());
        return file.write(bytes.data(), bytes.size());
    };
    const std::uint8_t header[] = {
        'M', 'G', 'R', 'F', 1, static_cast<std::uint8_t>(startedAt.has_value()),
        static_cast<std::uint8_t>(channels.size())
    };
    if (!write(header)) return false;
    if (startedAt) {
        std::uint8_t utc[8];
        putLittleEndian(utc, static_cast<std::uint64_t>(startedAt->time_since_epoch().count()));
        if (!write(utc)) return false;
    }
    for (const auto& channel : channels) {
        std::uint8_t lengths[4];
        putLittleEndian(lengths, static_cast<std::uint16_t>(channel.id.size()));
        putLittleEndian(lengths + 2, static_cast<std::uint16_t>(channel.unit.size()));
        if (!write(lengths) ||
            !write({reinterpret_cast<const std::uint8_t*>(channel.id.data()), channel.id.size()}) ||
            !write({reinterpret_cast<const std::uint8_t*>(channel.unit.data()), channel.unit.size()})) {
            return false;
        }
    }

    std::uint8_t checksum[ChecksumBytes];
    putLittleEndian(checksum, crc.value());
    return file.write(checksum, sizeof(checksum));
}

bool writeBlock(io::File& file, std::uint8_t* block, std::size_t size) {
    const etl::crc32_t16 crc(block, block + size);
    putLittleEndian(block + size, crc.value());
    return file.write(block, size + ChecksumBytes);
}

} // namespace

RecordingManager::RecordingManager(io::FileSystem& fs) : fs_(fs) {}

RecordingManager::~RecordingManager() = default;

RecordingManager::Error RecordingManager::start(
    std::span<const Channel> channels, std::optional<std::chrono::sys_seconds> startedAt
) {
    constexpr const char* TAG = "RecordingManager::start";

    if (file_) {
        LOG_WARN(TAG, "Already recording.");
        return Error::AlreadyRecording;
    }

    if (!validChannels(channels)) {
        LOG_WARN(TAG, "Invalid channels detected.");
        return Error::InvalidChannels;
    }

    std::string path;
    std::unique_ptr<io::File> file;
    try {
        const auto utcStem = startedAt ? utcFilenameStem(*startedAt) : std::string{};
        std::uint32_t suffix = 0;
        auto& number = utcStem.empty() ? nextFallbackId_ : suffix;
        if (utcStem.empty() && number == 0) {
            LOG_ERROR(TAG, "Fallback recording filename IDs exhausted.");
            return failure_ = Error::StartFailed;
        }

        do {
            path = recordingPath(utcStem, number);
            auto created = fs_.create(path);
            if (!created.file && !created.alreadyExists){
                LOG_ERROR(TAG, "Failed to create recording file: %s", path.c_str());
                break;
            }

            ++number;

            if (!created.file) continue;
            file = std::move(created.file);
            if (!writeHeader(*file, channels, startedAt)) {
                LOG_ERROR(TAG, "Failed to write recording header: %s", path.c_str());
                break;
            }

            file_ = std::move(file);
            channelCount_ = channels.size();
            lastElapsed_ = {};
            failure_ = Error::None;

            LOG_INFO(TAG, "Recording started: %s (%u channels)", path.c_str(), static_cast<unsigned>(channelCount_));

            return Error::None;
        } while (number != 0);
    } catch (const std::bad_alloc&) {
        LOG_ERROR(TAG, "Memory allocation failed while starting recording.");
    }

    if (file) {
        file.reset();
        try {
            if (!fs_.remove(path)) {
                LOG_WARN(TAG, "Failed to remove incomplete recording: %s", path.c_str());
            }
        } catch (const std::bad_alloc&) {
            LOG_WARN(TAG, "Allocation failure during recording cleanup.");
        }
    }

    return failure_ = Error::StartFailed;
}

RecordingManager::Error RecordingManager::appendSamples(std::span<const Sample> samples) {
    constexpr const char* TAG = "RecordingManager::appendSamples";
    
    if (!file_) {
        LOG_WARN(TAG, "Cannot append samples: no active recording.");
        return Error::NotRecording;
    }

    auto elapsed = lastElapsed_;
    for (const auto& sample : samples) {
        if (sample.channelIndex >= channelCount_) {
            LOG_WARN(TAG, "Invalid sample channel index: %u", static_cast<unsigned>(sample.channelIndex));
            return Error::InvalidSamples;
        }

        if (sample.elapsed < elapsed) {
            LOG_WARN(TAG, "Out-of-order sample timestamp: %lld us (previous: %lld us)",
                static_cast<long long>(sample.elapsed.count()),
                static_cast<long long>(elapsed.count()));
            return Error::InvalidSamples;
        }

        if (!(std::abs(sample.value) <= MaxValue)) {
            LOG_WARN(TAG, "Sample contains an invalid measurement.");
            return Error::InvalidSamples;
        }

        elapsed = sample.elapsed;
    }

    std::uint8_t block[BlockBytes];
    while (!samples.empty()) {
        const auto firstElapsed = static_cast<std::uint64_t>(samples.front().elapsed.count());
        putLittleEndian(block + 1, firstElapsed);

        std::size_t count = 0;
        for (const auto& sample : samples.first(std::min(samples.size(), SamplesPerBlock))) {
            // Validation guarantees nonnegative, ordered timestamps.
            const auto offset = static_cast<std::uint64_t>(sample.elapsed.count()) - firstElapsed;
            if (offset > MaxOffset) break;

            auto* record = block + BlockHeaderBytes + count * SampleBytes;

            putLittleEndian(record, static_cast<std::uint32_t>(offset));
            record[4] = static_cast<std::uint8_t>(sample.channelIndex);
            putLittleEndian(record + 5, std::bit_cast<std::uint32_t>(static_cast<float>(sample.value)));

            ++count;
        }

        block[0] = static_cast<std::uint8_t>(count);
        if (!writeBlock(*file_, block, BlockHeaderBytes + count * SampleBytes)) {
            LOG_ERROR(TAG, "Failed to write sample block at %llu us (%u samples)",
                static_cast<unsigned long long>(firstElapsed),
                static_cast<unsigned>(count)
            );

            file_.reset();
            return failure_ = Error::WriteFailed;
        }

        samples = samples.subspan(count);
    }

    lastElapsed_ = elapsed;
    return Error::None;
}

RecordingManager::Error RecordingManager::flush() {
    constexpr const char* TAG = "RecordingManager::flush";

    if (!file_) {
        LOG_WARN(TAG, "Not recording.");
        return Error::NotRecording;
    }

    if (!file_->flush()) {
        LOG_ERROR(TAG, "Failed to flush recording to storage.");
        file_.reset();
        return failure_ = Error::FlushFailed;
    }

    return Error::None;
}

RecordingManager::Error RecordingManager::stop(std::chrono::microseconds duration) {
    constexpr const char* TAG = "RecordingManager::stop";

    if (!file_) {
        LOG_WARN(TAG, "Not recording.");
        return Error::NotRecording;
    }

    if (duration < lastElapsed_) {
        LOG_WARN(TAG, "Invalid recording duration: %lld us, last sample: %lld us",
            static_cast<long long>(duration.count()),
            static_cast<long long>(lastElapsed_.count())
        );
        return Error::InvalidDuration;
    }

    std::uint8_t block[BlockHeaderBytes + ChecksumBytes] = {};
    putLittleEndian(block + 1, static_cast<std::uint64_t>(duration.count()));
    const bool written = writeBlock(*file_, block, BlockHeaderBytes);
    const bool flushed = written && file_->flush();
    const bool closed = file_->close();
    file_.reset();

    if (!written || !flushed || !closed) {
        LOG_ERROR(TAG, "Failed to finalize recording (write=%d, flush=%d, close=%d)",
            static_cast<int>(written),
            static_cast<int>(flushed),
            static_cast<int>(closed)
        );
        return failure_ = Error::FinalizeFailed;
    }

    LOG_INFO(TAG, "Recording stopped successfully (duration=%lld us, channels=%u)",
        static_cast<long long>(duration.count()),
        static_cast<unsigned>(channelCount_)
    );

    return Error::None;
}

RecordingManager::Status RecordingManager::status() const noexcept {
    return {file_ != nullptr, failure_};
}

} // namespace mg
