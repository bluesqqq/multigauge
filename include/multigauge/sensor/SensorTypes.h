#pragma once

#include <cstdint>
#include <optional>

#include <multigauge/value/UnitType.h>

namespace mg {

/// Describes the state of a sensor's cached reading.
///
/// `Unavailable` means the sensor currently has no usable sample. That can
/// happen before the first successful poll, when a source is disconnected, or
/// when the provider intentionally withholds data for that cycle.
///
/// `Error` means the provider attempted to refresh the sensor and the sensor
/// could not produce a valid sample.
enum class SensorStatus : std::uint8_t {
    Unavailable,
    Available,
    Error
};

/// Cached output from a sensor.
struct SensorReading {
    Measurement value = 0.0F;
    SensorStatus status = SensorStatus::Unavailable;

    [[nodiscard]]
    constexpr bool available() const noexcept {
        return status == SensorStatus::Available;
    }

    [[nodiscard]]
    constexpr bool unavailable() const noexcept {
        return status == SensorStatus::Unavailable;
    }

    [[nodiscard]]
    constexpr bool error() const noexcept {
        return status == SensorStatus::Error;
    }
};

/// Native bounds reported by a sensor.
/// Either side may be unknown.
struct SensorRange {
    std::optional<Measurement> minimum;
    std::optional<Measurement> maximum;

    [[nodiscard]]
    constexpr bool hasMinimum() const noexcept {
        return minimum.has_value();
    }

    [[nodiscard]]
    constexpr bool hasMaximum() const noexcept {
        return maximum.has_value();
    }

    [[nodiscard]]
    constexpr bool empty() const noexcept {
        return !minimum.has_value() && !maximum.has_value();
    }
};

} // namespace mg
