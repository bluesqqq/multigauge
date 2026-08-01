#pragma once

#include <chrono>
#include <cstddef>
#include <string_view>

#include <multigauge/sensor/Sensor.h>

namespace mg {

/// Owns a group of related sensors and refreshes them from shared hardware.
///
/// Providers are responsible for polling, decoding, and cache invalidation.
/// A single provider may update multiple logical sensors from one hardware
/// transaction, so the caller only asks the provider to refresh once per cycle.
class SensorProvider {
public:
    virtual ~SensorProvider() = default;

    /// Stable identifier for registry/editor use.
    /// The returned view must remain valid for the lifetime of the provider.
    [[nodiscard]]
    virtual std::string_view id() const noexcept = 0;

    /// Human-readable display name.
    /// The returned view must remain valid for the lifetime of the provider.
    [[nodiscard]]
    virtual std::string_view name() const noexcept = 0;

    /// Refreshes all supplied sensors from the provider's backing hardware.
    ///
    /// Providers update each sensor's cached reading directly. Partial success
    /// is represented by the individual sensor statuses, not by a summary
    /// return value. `elapsed` is the delta time since the provider's previous
    /// update call.
    virtual void update(std::chrono::microseconds elapsed) noexcept = 0;

    /// Number of sensors currently supplied by the provider.
    [[nodiscard]]
    virtual std::size_t sensorCount() const noexcept = 0;

    /// Returns a borrowed sensor pointer for the given index, or nullptr.
    [[nodiscard]]
    virtual const Sensor* sensorAt(std::size_t index) const noexcept = 0;
};

} // namespace mg
