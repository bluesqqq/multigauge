#pragma once

#include <chrono>

namespace mg::io {

class Time {
    public:
        virtual ~Time() = default;

        /// Monotonic elapsed time; the epoch is intentionally unspecified.
        virtual std::chrono::microseconds elapsed() const = 0;
};

} // namespace mg::io
