#include <doctest/doctest.h>
#include <lodepng.h>

#include <multigauge/runtime/RecordingManager.h>

#include <bit>
#include <limits>

#include "MemoryFileSystem.h"

namespace {

using Manager = mg::RecordingManager;
using Error = Manager::Error;
using Bytes = std::span<const std::uint8_t>;
using namespace std::chrono_literals;

constexpr Manager::Channel Channels[] = {{"rpm", "rpm"}, {"coolant", "C"}, {"user.aux_voltage", "V"}};
constexpr auto FirstPath = "/recordings/recording_000001.mgr";
constexpr auto SecondPath = "/recordings/recording_000002.mgr";

std::uint64_t readLE(Bytes bytes, std::size_t offset, std::size_t length) {
    std::uint64_t value = 0;
    for (std::size_t i = 0; i < length; ++i) {
        value |= static_cast<std::uint64_t>(bytes[offset + i]) << (8 * i);
    }
    return value;
}

bool validCrc(Bytes bytes, std::size_t payloadLength) {
    return lodepng_crc32(bytes.data(), payloadLength) == readLE(bytes, payloadLength, 4);
}

struct DecodedRecording {
    std::optional<std::int64_t> utcSeconds;
    std::vector<std::pair<std::string, std::string>> channels;
    std::vector<Manager::Sample> samples;
    std::optional<std::chrono::microseconds> duration;
    std::vector<std::size_t> blockCounts;
};

// Test-only decoder of the documented wire format, with an independent CRC.
bool readHeader(Bytes& bytes, DecodedRecording& recording) {
    if (bytes.size() < 7 || std::string_view(reinterpret_cast<const char*>(bytes.data()), 4) != "MGRF" ||
        bytes[4] != 1 || bytes[5] > 1 || bytes[6] == 0) return false;

    std::size_t pos = 7;
    if (bytes[5] == 1) {
        if (bytes.size() < pos + 8) return false;
        recording.utcSeconds = std::bit_cast<std::int64_t>(readLE(bytes, pos, 8));
        pos += 8;
    }
    for (unsigned i = 0; i < bytes[6]; ++i) {
        if (bytes.size() < pos + 4) return false;
        const auto idLength = static_cast<std::size_t>(readLE(bytes, pos, 2));
        const auto unitLength = static_cast<std::size_t>(readLE(bytes, pos + 2, 2));
        pos += 4;
        if (bytes.size() < pos + idLength + unitLength) return false;
        const auto* id = reinterpret_cast<const char*>(bytes.data() + pos);
        recording.channels.emplace_back(std::string(id, idLength), std::string(id + idLength, unitLength));
        pos += idLength + unitLength;
    }
    if (bytes.size() < pos + 4 || !validCrc(bytes, pos)) return false;
    bytes = bytes.subspan(pos + 4);
    return true;
}

DecodedRecording decode(Bytes bytes) {
    DecodedRecording recording;
    REQUIRE(readHeader(bytes, recording));
    while (!bytes.empty()) {
        const std::size_t count = bytes[0];
        const std::size_t payloadLength = 9 + count * 9;
        if (count > 55 || bytes.size() < payloadLength + 4 || !validCrc(bytes, payloadLength)) break;

        const auto elapsed = readLE(bytes, 1, 8);
        if (count == 0) {
            recording.duration = std::chrono::microseconds(elapsed);
            break;
        }
        recording.blockCounts.push_back(count);
        for (std::size_t i = 0; i < count; ++i) {
            const auto offset = 9 + i * 9;
            const auto time = std::chrono::microseconds(elapsed + readLE(bytes, offset, 4));
            const auto value = std::bit_cast<float>(static_cast<std::uint32_t>(readLE(bytes, offset + 5, 4)));
            recording.samples.push_back({time, bytes[offset + 4], value});
        }
        bytes = bytes.subspan(payloadLength + 4);
    }
    return recording;
}

TEST_CASE("recording stores mixed-rate updates, base unit metadata and the actual duration") {
    mg::test::MemoryFileSystem fs;
    Manager manager(fs);
    const std::chrono::sys_seconds rtcStart{1700000000s};
    REQUIRE(manager.start(Channels, rtcStart) == Error::None);
    CHECK(manager.status().recording);
    CHECK(manager.start(Channels) == Error::AlreadyRecording);
    const Manager::Sample initial[] = {{0us, 0, 1000}, {0us, 1, -2.5F}, {0us, 2, 12.25F}};
    const Manager::Sample later[] = {{20ms, 0, 1200}, {40ms, 0, 1250}, {500ms, 1, -2.25F}};
    REQUIRE(manager.appendSamples(initial) == Error::None);
    REQUIRE(manager.appendSamples({}) == Error::None);
    REQUIRE(manager.appendSamples(later) == Error::None);
    REQUIRE(manager.stop(3s) == Error::None);
    CHECK_FALSE(manager.status().recording);
    CHECK(manager.status().failure == Error::None);
    CHECK(fs.closeCalls == 1);

    const auto recording = decode(fs.files.at("/recordings/2023-11-14_22-13-20.mgr"));
    CHECK(recording.utcSeconds == 1700000000);
    REQUIRE(recording.channels.size() == std::size(Channels));
    for (std::size_t i = 0; i < recording.channels.size(); ++i) {
        CHECK(recording.channels[i].first == Channels[i].id);
        CHECK(recording.channels[i].second == Channels[i].unit);
    }
    CHECK(recording.duration == 3s);
    CHECK(recording.blockCounts == std::vector<std::size_t>{3, 3});
    const Manager::Sample expected[] = {initial[0], initial[1], initial[2], later[0], later[1], later[2]};
    REQUIRE(recording.samples.size() == std::size(expected));
    for (std::size_t i = 0; i < recording.samples.size(); ++i) {
        CHECK(recording.samples[i].elapsed == expected[i].elapsed);
        CHECK(recording.samples[i].channelIndex == expected[i].channelIndex);
        CHECK(recording.samples[i].value == expected[i].value);
    }
}

TEST_CASE("recording filenames use UTC and preserve collisions across restarts") {
    using namespace std::chrono;
    mg::test::MemoryFileSystem fs;
    const sys_seconds start = sys_days{2026y/November/10} + 8h + 45min + 32s;
    const char* paths[] = {
        "/recordings/2026-11-10_08-45-32.mgr",
        "/recordings/2026-11-10_08-45-32_001.mgr",
        "/recordings/2026-11-10_08-45-32_002.mgr",
    };
    for (std::size_t i = 0; i < std::size(paths); ++i) {
        const auto existing = fs.files;
        Manager manager(fs);
        REQUIRE(manager.start(Channels, start) == Error::None);
        REQUIRE(manager.stop(seconds(i)) == Error::None);
        for (const auto& [path, bytes] : existing) CHECK(fs.files.at(path) == bytes);
        const auto recording = decode(fs.files.at(paths[i]));
        CHECK(recording.utcSeconds == start.time_since_epoch().count());
        CHECK(recording.duration == seconds(i));
    }
    CHECK(fs.directories.contains("/recordings"));
}

TEST_CASE("recording fallback filenames skip occupied names after a restart") {
    mg::test::MemoryFileSystem fs;
    fs.files[FirstPath] = {}; // An occupied name can be an empty file after a power cut.
    {
        Manager manager(fs);
        REQUIRE(manager.start(Channels) == Error::None);
        REQUIRE(manager.stop(0us) == Error::None);
    }
    const auto saved = fs.files.at(SecondPath);
    const auto empty = decode(saved);
    CHECK_FALSE(empty.utcSeconds.has_value());
    CHECK(empty.samples.empty());
    CHECK(empty.duration == 0us);

    Manager manager(fs);
    const std::chrono::sys_seconds outOfRange = std::chrono::sys_days{std::chrono::year{10000}/1/1};
    REQUIRE(manager.start(Channels, outOfRange) == Error::None);
    REQUIRE(manager.stop(1s) == Error::None);
    CHECK(fs.files.at(FirstPath).empty());
    CHECK(fs.files.at(SecondPath) == saved);
    CHECK(decode(fs.files.at("/recordings/recording_000003.mgr")).utcSeconds ==
          outOfRange.time_since_epoch().count());
}

TEST_CASE("recording splits large batches and long gaps without losing elapsed precision") {
    mg::test::MemoryFileSystem fs;
    Manager manager(fs);
    REQUIRE(manager.start(Channels) == Error::None);
    std::vector<Manager::Sample> samples(55, {2us, 0, 1000});
    constexpr auto MaxOffset = 4294967295us;
    samples.push_back({3us, 1, 85});
    samples.push_back({3us + MaxOffset, 2, 13});
    samples.push_back({4us + MaxOffset, 0, 1100});
    REQUIRE(manager.appendSamples(samples) == Error::None);
    REQUIRE(manager.stop(2h) == Error::None);
    Bytes bytes = fs.files.at(FirstPath);
    DecodedRecording header;
    REQUIRE(readHeader(bytes, header));
    REQUIRE(bytes.size() == 508 + 31 + 22 + 13);
    CHECK(bytes[508] == 2);
    const auto recording = decode(fs.files.at(FirstPath));
    CHECK(recording.blockCounts == std::vector<std::size_t>{55, 2, 1});
    REQUIRE(recording.samples.size() == samples.size());
    CHECK(recording.samples[56].elapsed == 3us + MaxOffset);
    CHECK(recording.samples[57].elapsed == 4us + MaxOffset);
    CHECK(recording.duration == 2h);
}

TEST_CASE("recording encodes maximum elapsed timestamps and binary32 values") {
    mg::test::MemoryFileSystem fs;
    Manager manager(fs);
    const std::string longId(128, 'i');
    const std::string longUnit(32, 'u');
    const Manager::Channel channels[] = {{longId, longUnit}};
    REQUIRE(manager.start(channels) == Error::None);
    constexpr auto end = std::chrono::microseconds::max();
    const Manager::Sample samples[] = {
        {0us, 0, 1.25F}, {end - 1us, 0, -0.0F},
        {end, 0, std::numeric_limits<float>::denorm_min()},
        {end, 0, std::numeric_limits<float>::max()},
    };
    REQUIRE(manager.appendSamples(samples) == Error::None);

    if constexpr (std::numeric_limits<mg::Measurement>::digits > std::numeric_limits<float>::digits) {
        const Manager::Sample wider[] = {
            {end, 0, 1 + std::numeric_limits<mg::Measurement>::epsilon()},
            {end, 0, std::numeric_limits<mg::Measurement>::min()},
        };
        REQUIRE(manager.appendSamples(wider) == Error::None);
    }
    REQUIRE(manager.stop(end) == Error::None);

    const auto& saved = fs.files.at(FirstPath);
    const auto recording = decode(saved);
    CHECK(recording.duration == end);
    REQUIRE(recording.samples.size() >= 4);
    CHECK(recording.samples[1].elapsed == end - 1us);
    CHECK(recording.samples[2].elapsed == end);
    REQUIRE(recording.channels.size() == 1);
    CHECK(recording.channels[0].first == longId);
    CHECK(recording.channels[0].second == longUnit);

    Bytes bytes = saved;
    DecodedRecording header;
    REQUIRE(readHeader(bytes, header));
    REQUIRE(bytes[0] == 1);
    CHECK(readLE(bytes, 14, 4) == 0x3fa00000u); // 1.25
    bytes = bytes.subspan(22);
    REQUIRE(bytes[0] == 3);
    CHECK(readLE(bytes, 14, 4) == 0x80000000u); // Negative zero
    CHECK(readLE(bytes, 23, 4) == 0x00000001u); // Smallest subnormal
    CHECK(readLE(bytes, 32, 4) == 0x7f7fffffu); // Largest finite binary32
    if constexpr (std::numeric_limits<mg::Measurement>::digits > std::numeric_limits<float>::digits) {
        bytes = bytes.subspan(40);
        REQUIRE(bytes[0] == 2);
        CHECK(readLE(bytes, 14, 4) == 0x3f800000u); // Rounded to 1
        CHECK(readLE(bytes, 23, 4) == 0u); // Underflow to zero
    }
}

TEST_CASE("recording rejects invalid channels, batches and duration before writing") {
    mg::test::MemoryFileSystem fs;
    Manager manager(fs);
    const Manager::Channel duplicate[] = {{"same", "V"}, {"same", "C"}};
    const Manager::Channel unnamed[] = {{"", "V"}};
    const std::string longLabel(33, 'x');
    const std::string longId(129, 'x');
    const Manager::Channel oversized[] = {{"rpm", longLabel}};
    const Manager::Channel oversizedId[] = {{longId, "rpm"}};
    const std::vector<Manager::Channel> tooMany(256, {"rpm", "rpm"});
    CHECK(manager.start({}) == Error::InvalidChannels);
    CHECK(manager.start(duplicate) == Error::InvalidChannels);
    CHECK(manager.start(unnamed) == Error::InvalidChannels);
    CHECK(manager.start(oversized) == Error::InvalidChannels);
    CHECK(manager.start(oversizedId) == Error::InvalidChannels);
    CHECK(manager.start(tooMany) == Error::InvalidChannels);
    CHECK(manager.appendSamples({}) == Error::NotRecording);
    CHECK(manager.stop(0us) == Error::NotRecording);
    CHECK(fs.files.empty());

    REQUIRE(manager.start(Channels) == Error::None);
    const Manager::Sample negative[] = {{-1us, 0, 1000}};
    CHECK(manager.appendSamples(negative) == Error::InvalidSamples);
    CHECK(manager.stop(-1us) == Error::InvalidDuration);
    const Manager::Sample first[] = {{20ms, 0, 1000}};
    REQUIRE(manager.appendSamples(first) == Error::None);
    const auto saved = fs.files.at(FirstPath);
    const auto inf = std::numeric_limits<mg::Measurement>::infinity();
    const auto nan = std::numeric_limits<mg::Measurement>::quiet_NaN();
    const Manager::Sample invalid[] = {{1s, 3, 0}, {1s, 1, inf}, {1s, 1, nan}, {999ms, 1, 85}};
    for (const auto& sample : invalid) {
        std::vector<Manager::Sample> batch(56, {1s, 0, 1200});
        batch.push_back(sample);
        CHECK(manager.appendSamples(batch) == Error::InvalidSamples);
        CHECK(fs.files.at(FirstPath) == saved);
    }
    if constexpr (std::numeric_limits<mg::Measurement>::max_exponent > std::numeric_limits<float>::max_exponent) {
        const Manager::Sample outOfRange[] = {{1s, 0, std::numeric_limits<mg::Measurement>::max()}};
        CHECK(manager.appendSamples(outOfRange) == Error::InvalidSamples);
    }
    const Manager::Sample older[] = {{19ms, 0, 900}};
    CHECK(manager.appendSamples(older) == Error::InvalidSamples);
    CHECK(manager.stop(19ms) == Error::InvalidDuration);
    CHECK(fs.files.at(FirstPath) == saved);
    CHECK(manager.status().recording);
    CHECK(manager.status().failure == Error::None);
    REQUIRE(manager.stop(20ms) == Error::None);
}

TEST_CASE("recording recovers complete blocks after interruptions and checksum failures") {
    mg::test::MemoryFileSystem fs;
    std::size_t firstEnd = 0;
    std::size_t secondEnd = 0;
    {
        Manager manager(fs);
        REQUIRE(manager.start(Channels) == Error::None);
        const Manager::Sample first[] = {{0us, 0, 1000}};
        const Manager::Sample second[] = {{20ms, 0, 1100}, {20ms, 1, 85}};
        REQUIRE(manager.appendSamples(first) == Error::None);
        firstEnd = fs.files.at(FirstPath).size();
        REQUIRE(manager.appendSamples(second) == Error::None);
        secondEnd = fs.files.at(FirstPath).size();
        SUBCASE("interrupted before finalization") {}
        SUBCASE("finalized before truncation") { REQUIRE(manager.stop(1s) == Error::None); }
    }

    const auto& saved = fs.files.at(FirstPath);
    for (std::size_t cut = firstEnd; cut < saved.size(); ++cut) {
        const auto recovered = decode(Bytes(saved).first(cut));
        REQUIRE(recovered.samples.size() == (cut < secondEnd ? 1 : 3));
        CHECK(recovered.samples.front().value == 1000);
        CHECK_FALSE(recovered.duration.has_value());
    }
    auto corrupted = saved;
    corrupted[secondEnd - 5] ^= 1; // Change the last sample byte, preserving framing.
    const auto recovered = decode(corrupted);
    CHECK(recovered.samples.size() == 1);
    CHECK_FALSE(recovered.duration.has_value());
    if (saved.size() == secondEnd) {
        const auto interrupted = decode(saved);
        CHECK(interrupted.samples.size() == 3);
        CHECK_FALSE(interrupted.duration.has_value());
    }
}

TEST_CASE("recording header validation rejects truncated metadata and a bad checksum") {
    mg::test::MemoryFileSystem fs;
    Manager manager(fs);
    constexpr auto path = "/recordings/1970-01-01_00-00-00.mgr";
    REQUIRE(manager.start(Channels, std::chrono::sys_seconds{}) == Error::None);
    auto header = fs.files.at(path);
    Bytes complete = header;
    DecodedRecording recording;
    REQUIRE(readHeader(complete, recording));
    CHECK(complete.empty());
    for (std::size_t cut = 0; cut < header.size(); ++cut) {
        auto truncated = Bytes(header).first(cut);
        DecodedRecording partial;
        CHECK_FALSE(readHeader(truncated, partial));
    }
    header.back() ^= 1;
    Bytes corrupted = header;
    CHECK_FALSE(readHeader(corrupted, recording));
}

TEST_CASE("recording reports failed opens and writes and clears failure on a successful start") {
    mg::test::MemoryFileSystem fs;
    Manager manager(fs);
    fs.failDirectory = true;
    CHECK(manager.start(Channels) == Error::StartFailed);
    CHECK(fs.files.empty());
    fs.failDirectory = false;
    fs.failOpen = true;
    CHECK(manager.start(Channels) == Error::StartFailed);
    CHECK_FALSE(manager.status().recording);
    CHECK(manager.status().failure == Error::StartFailed);
    CHECK(fs.files.empty());
    fs.failOpen = false;
    fs.writeBudget = 8;
    CHECK(manager.start(Channels) == Error::StartFailed);
    CHECK_FALSE(manager.status().recording);
    CHECK(fs.files.empty());
    CHECK(fs.directories.contains("/recordings"));
    CHECK(fs.closeCalls == 1);

    fs.writeBudget.reset();
    REQUIRE(manager.start(Channels) == Error::None);
    CHECK(manager.status().failure == Error::None);
    const Manager::Sample initial[] = {{0us, 0, 1000}};
    REQUIRE(manager.appendSamples(initial) == Error::None);
    fs.writeBudget = 55 * 9 + 13 + 10; // One complete block and a partial second block.
    const std::vector<Manager::Sample> later(56, {20ms, 0, 1100});
    CHECK(manager.appendSamples(later) == Error::WriteFailed);
    CHECK_FALSE(manager.status().recording);
    CHECK(manager.stop(1s) == Error::NotRecording);
    CHECK(manager.status().failure == Error::WriteFailed);
    CHECK(fs.closeCalls == 2);
    const auto recovered = decode(fs.files.at(SecondPath));
    CHECK(recovered.blockCounts == std::vector<std::size_t>{1, 55});
    CHECK_FALSE(recovered.duration.has_value());
}

TEST_CASE("recording preserves occupied paths even if the existence query misses them") {
    class HiddenRecordingFileSystem : public mg::test::MemoryFileSystem {
        bool exists(const std::string& path) override {
            return path != FirstPath && MemoryFileSystem::exists(path);
        }
    } fs;
    fs.files[FirstPath] = {};
    SUBCASE("empty existing file") {}
    SUBCASE("nonempty existing file") { fs.files[FirstPath] = {1, 2, 3}; }
    const auto saved = fs.files.at(FirstPath);
    Manager manager(fs);
    REQUIRE(manager.start(Channels) == Error::None);
    CHECK(manager.stop(1s) == Error::None);
    CHECK(fs.files.at(FirstPath) == saved);
    CHECK(decode(fs.files.at(SecondPath)).duration == 1s);
}

TEST_CASE("recording cleanup preserves occupied names and the original start failure") {
    mg::test::MemoryFileSystem fs;
    fs.files[FirstPath] = {1, 2, 3};
    fs.writeBudget = 8;
    SUBCASE("cleanup succeeds even if close reports an error") { fs.failClose = true; }
    SUBCASE("cleanup cannot remove the partial header") { fs.failRemove = true; }
    {
        Manager manager(fs);
        CHECK(manager.start(Channels) == Error::StartFailed);
        CHECK_FALSE(manager.status().recording);
        CHECK(manager.status().failure == Error::StartFailed);
    }
    CHECK(fs.closeCalls == 1);
    CHECK(fs.files.at(FirstPath) == std::vector<std::uint8_t>{1, 2, 3});
    CHECK(fs.files.contains(SecondPath) == fs.failRemove);
    if (fs.failRemove) {
        auto partial = Bytes(fs.files.at(SecondPath));
        CHECK(partial.size() == 8);
        DecodedRecording recording;
        CHECK_FALSE(readHeader(partial, recording));
    }

    // After a reboot, only the name of a successfully removed file can be reused.
    fs.writeBudget.reset();
    fs.failClose = false;
    Manager restarted(fs);
    REQUIRE(restarted.start(Channels) == Error::None);
    REQUIRE(restarted.stop(0us) == Error::None);
    const auto* newPath = fs.failRemove ? "/recordings/recording_000003.mgr" : SecondPath;
    CHECK(decode(fs.files.at(newPath)).duration == 0us);
    if (fs.failRemove) CHECK(fs.files.at(SecondPath).size() == 8);
}

TEST_CASE("recording flushes only on request and reports backend failure") {
    mg::test::MemoryFileSystem fs;
    Manager manager(fs);
    CHECK(manager.flush() == Error::NotRecording);
    REQUIRE(manager.start(Channels) == Error::None);
    const Manager::Sample first[] = {{0us, 0, 1000}};
    REQUIRE(manager.appendSamples(first) == Error::None);
    CHECK(fs.flushCalls == 0);
    const auto checkpoint = fs.files.at(FirstPath);
    REQUIRE(manager.flush() == Error::None);
    CHECK(fs.flushCalls == 1);
    CHECK(fs.files.at(FirstPath) == checkpoint);
    CHECK(manager.status().recording);
    const auto recovered = decode(checkpoint);
    REQUIRE(recovered.samples.size() == 1);
    CHECK_FALSE(recovered.duration.has_value());

    const Manager::Sample later[] = {{20ms, 0, 1200}};
    REQUIRE(manager.appendSamples(later) == Error::None);
    CHECK(fs.flushCalls == 1);
    fs.failFlush = true;
    CHECK(manager.flush() == Error::FlushFailed);
    CHECK(fs.flushCalls == 2);
    CHECK(fs.closeCalls == 1);
    CHECK_FALSE(manager.status().recording);
    CHECK(manager.flush() == Error::NotRecording);
    CHECK(manager.status().failure == Error::FlushFailed);
    CHECK_FALSE(decode(fs.files.at(FirstPath)).duration.has_value());

    fs.failFlush = false;
    REQUIRE(manager.start(Channels) == Error::None);
    CHECK(manager.status().failure == Error::None);
    CHECK(manager.stop(1s) == Error::None);
}

TEST_CASE("recording reports finalization write, flush and close failures and releases its file") {
    mg::test::MemoryFileSystem fs;
    Manager manager(fs);
    REQUIRE(manager.start(Channels) == Error::None);
    const Manager::Sample first[] = {{0us, 0, 1000}};
    REQUIRE(manager.appendSamples(first) == Error::None);
    SUBCASE("partial finalization record") { fs.writeBudget = 6; }
    SUBCASE("flush failure") { fs.failFlush = true; }
    SUBCASE("close failure") { fs.failClose = true; }
    CHECK(manager.stop(1s) == Error::FinalizeFailed);
    CHECK_FALSE(manager.status().recording);
    CHECK(manager.status().failure == Error::FinalizeFailed);
    CHECK(fs.closeCalls == 1);
    CHECK(decode(fs.files.at(FirstPath)).samples.size() == 1);
}

} // namespace
