#pragma once

#include <chrono>
#include <cstddef>
#include <string_view>

#include <multigauge/sensor/Sensor.h>
#include <multigauge/json/Json.h>

namespace mg::sensor {

/// Owns a group of related sensors and refreshes them from shared hardware.
///
/// Providers are responsible for polling, decoding, and cache invalidation.
/// A single provider may update multiple logical sensors from one hardware
/// transaction, so the caller only asks the provider to refresh once per cycle.
class Provider {
public:
    virtual ~Provider() = default;

    //----------[ IDENTITY ]----------//
    
    /// Stable identifier for registry/editor use.
    /// The returned view must remain valid for the lifetime of the provider.
    [[nodiscard]]
    virtual std::string_view id() const noexcept = 0;

    /// Human-readable display name.
    /// The returned view must remain valid for the lifetime of the provider.
    [[nodiscard]]
    virtual std::string_view name() const noexcept = 0;

    /// Stable implementation type used by persisted core configuration.
    [[nodiscard]] virtual std::string_view type() const noexcept { return "custom"; }

    //----------[ UPDATE ]----------//

    /// Refreshes all supplied sensors from the provider's backing hardware.
    ///
    /// Providers update each sensor's cached reading directly. Partial success
    /// is represented by the individual sensor statuses, not by a summary
    /// return value. `elapsed` is the delta time since the provider's previous
    /// update call.
    virtual void update(std::chrono::microseconds elapsed) noexcept = 0;

    //----------[ SENSORS ]----------//

    /// Number of sensors currently supplied by the provider.
    [[nodiscard]]
    virtual std::size_t sensorCount() const noexcept = 0;

    /// Returns a borrowed sensor pointer for the given index, or nullptr.
    [[nodiscard]]
    virtual const Sensor* sensorAt(std::size_t index) const noexcept = 0;

    //----------[ CONFIGURATION ]----------//

    /// Applies provider-specific persisted configuration. The core treats the
    /// document as opaque; implementations validate their own fields. A false
    /// result must leave the provider unchanged. A successful call may change
    /// its sensor set; Manager will reindex it before accepting the change.
    [[nodiscard]] virtual bool loadConfiguration(json::Reader config) {
        (void)config;
        return true;
    }

    /// Writes provider-specific persisted configuration as one JSON object.
    /// The emitted object must be accepted later by loadConfiguration so the
    /// manager can restore the provider if a reconfiguration cannot be indexed.
    [[nodiscard]] virtual bool saveConfiguration(json::Writer& writer) const {
        return writer.writeObject([](json::ObjectWriter&) { return true; });
    }
};

} // namespace mg::sensor
