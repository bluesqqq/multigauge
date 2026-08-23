#pragma once

#include <cstdint>
#include <optional>

#include <multigauge/value/UnitType.h>

namespace mg::sensor {

/// @brief Describes a sensor reading's availability state.
enum class Status : std::uint8_t {
    Unavailable,
    Available,
    Error
};

/// @brief Cached output from a sensor.
struct Reading {
    Measurement value = 0.0F;
    Status status = Status::Unavailable;
    std::uint32_t sequence = 0;

    /// @brief Reports whether this reading has a usable sample.
    /// @return True when status is Status::Available.
    [[nodiscard]]
    constexpr bool available() const noexcept {
        return status == Status::Available;
    }

    /// @brief Reports whether this reading has no usable sample.
    /// @return True when status is Status::Unavailable.
    [[nodiscard]]
    constexpr bool unavailable() const noexcept {
        return status == Status::Unavailable;
    }

    /// @brief Reports whether the last refresh failed.
    /// @return True when status is Status::Error.
    [[nodiscard]]
    constexpr bool error() const noexcept {
        return status == Status::Error;
    }
};

/// @brief Native sensor bounds with optional limits.
struct Range {
    std::optional<Measurement> minimum;
    std::optional<Measurement> maximum;

    /// @brief Reports whether a lower bound is known.
    /// @return True when minimum has a value.
    [[nodiscard]]
    constexpr bool hasMinimum() const noexcept {
        return minimum.has_value();
    }

    /// @brief Reports whether an upper bound is known.
    /// @return True when maximum has a value.
    [[nodiscard]]
    constexpr bool hasMaximum() const noexcept {
        return maximum.has_value();
    }

    /// @brief Reports whether neither bound is known.
    /// @return True when both minimum and maximum are empty.
    [[nodiscard]]
    constexpr bool empty() const noexcept {
        return !minimum.has_value() && !maximum.has_value();
    }
};

} // namespace mg::sensor
