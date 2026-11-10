#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

#include <multigauge/value/Unit.h>

namespace mg::io { class File; class FileSystem; }

namespace mg {

/// @brief Manages one active recording of caller-supplied samples at a time.
/// @note Callers must serialize access, including status queries, and recording
/// creation within the destination directory. Start, append, flush, and stop can
/// block on filesystem I/O. A real-time host should call them from its storage
/// task and buffer batches outside this manager.
class RecordingManager {
public:
    /// @brief Describes one channel in the recording's fixed channel list.
    struct Channel {
        std::string_view id; ///< Nonempty identifier, unique within the recording.
        std::string_view unit{}; ///< Canonical base unit label, or empty for unitless values.
    };

    /// @brief One channel update. Callers may omit unchanged values.
    struct Sample {
        std::chrono::microseconds elapsed{}; ///< Monotonic time since the recording started.
        std::size_t channelIndex = 0; ///< Zero-based index into the channel list passed to start().
        Measurement value = 0.0F; ///< Value in the channel's canonical base unit.
    };

    enum class Error {
        None,
        AlreadyRecording,
        NotRecording,
        InvalidChannels,
        InvalidSamples,
        InvalidDuration,
        StartFailed,
        WriteFailed,
        FinalizeFailed,
        FlushFailed,
    };

    struct Status {
        bool recording = false; ///< True while samples can be appended.
        Error failure = Error::None; ///< Most recent storage failure since the last successful start.
    };

    /// @brief Borrows the destination filesystem. The manager starts idle.
    /// @param fs The host initializes this backend and keeps it alive until the
    /// manager is destroyed. The backend must support exclusive file creation.
    explicit RecordingManager(io::FileSystem& fs);

    /// @brief Releases recording resources. Call stop() to finalize explicitly.
    ~RecordingManager();

    RecordingManager(const RecordingManager&) = delete;
    RecordingManager& operator=(const RecordingManager&) = delete;

    /// @brief Opens a new recording with the given channels in the given order.
    /// @param channels One to 255 channels. Identifiers can occupy up to 128 bytes
    /// and unit labels up to 32 bytes. The list and its strings are borrowed for
    /// the call and written to the recording on success.
    /// @param startedAt Optional UTC start time, at second precision, corresponding
    /// to elapsed zero. Stored in the header and used for filenames when the year
    /// is in [1, 9999]. Sample ordering uses elapsed time.
    /// @return None once the destination is ready and the recording can accept
    /// samples. AlreadyRecording and InvalidChannels leave state unchanged.
    /// StartFailed leaves the manager idle and reports the failure in status().
    /// If the header write fails, the manager attempts to remove the new file.
    [[nodiscard]] Error start(
        std::span<const Channel> channels,
        std::optional<std::chrono::sys_seconds> startedAt = std::nullopt
    );

    /// @brief Appends a batch to the active recording.
    /// @param samples Borrowed for the call. Each sample must refer to a declared
    /// channel and have a finite value within the binary32 range. Elapsed times
    /// must be nonnegative and nondecreasing across all channels and batches.
    /// Equal timestamps preserve submission order.
    /// @return None once the whole batch has been written to the backend in order.
    /// The backend may still have buffered data.
    /// An empty batch succeeds while recording. NotRecording and InvalidSamples
    /// leave state and stored data unchanged. WriteFailed ends the recording,
    /// releases its resources, and may leave partial data on the destination.
    [[nodiscard]] Error appendSamples(std::span<const Sample> samples);

    /// @brief Flushes the active recording without finalizing it. The host chooses
    /// when to flush, typically every 1–5 seconds on its storage task.
    /// @return None when the backend reports success, subject to its durability
    /// guarantees. FlushFailed ends the recording, releases its resources, and
    /// is reported in status(). NotRecording leaves state unchanged.
    [[nodiscard]] Error flush();

    /// @brief Finalizes the active recording, even if it has no samples.
    /// @param duration Monotonic elapsed time at the end of the recording. It must
    /// be nonnegative and at least the last sample's elapsed time, if any.
    /// @return None after finalization, flush, and close succeed. This uses the
    /// backend's flush guarantees. FinalizeFailed may leave partial data. Both
    /// outcomes release the recording and leave the manager idle. NotRecording
    /// and InvalidDuration leave state unchanged.
    [[nodiscard]] Error stop(std::chrono::microseconds duration);

    /// @brief Returns the current state and any recorded storage failure without I/O.
    /// @details A storage failure remains visible while idle until a successful
    /// start clears it. Rejected calls leave the recorded failure unchanged.
    [[nodiscard]] Status status() const noexcept;

private:
    io::FileSystem& fs_;
    std::unique_ptr<io::File> file_;
    std::size_t channelCount_ = 0;
    std::chrono::microseconds lastElapsed_{};
    std::uint32_t nextFallbackId_ = 1;
    Error failure_ = Error::None;
};

} // namespace mg
