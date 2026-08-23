#pragma once

#include <chrono>
#include <cstddef>
#include <string_view>

#include <multigauge/sensor/Sensor.h>
#include <multigauge/json/Json.h>

namespace mg::sensor {

/// @brief Owns and refreshes a group of related sensors.
class Provider {
public:
    /// @brief Destroys the provider and its owned sensors.
    virtual ~Provider() = default;

    //----------[ IDENTITY ]----------//
    
    /// @brief Returns the stable identifier used by registries and persistence.
    /// @return A view valid for the lifetime of this provider.
    [[nodiscard]]
    virtual std::string_view id() const noexcept = 0;

    /// @brief Returns the human-readable display name.
    /// @return A view valid for the lifetime of this provider.
    [[nodiscard]]
    virtual std::string_view name() const noexcept = 0;

    /// @brief Returns the stable implementation type for persisted configuration.
    /// @return The provider type; defaults to "custom".
    [[nodiscard]] virtual std::string_view type() const noexcept { return "custom"; }

    //----------[ UPDATE ]----------//

    /// @brief Refreshes all supplied sensors from the backing hardware.
    /// @param elapsed Time since the previous update call.
    virtual void update(std::chrono::microseconds elapsed) noexcept = 0;

    //----------[ SENSORS ]----------//

    /// @brief Returns the number of currently supplied sensors.
    /// @return The number of sensors available through sensorAt().
    [[nodiscard]]
    virtual std::size_t sensorCount() const noexcept = 0;

    /// @brief Returns a borrowed sensor at an index.
    /// @param index The zero-based sensor index.
    /// @return The sensor, or nullptr when index is out of range.
    [[nodiscard]]
    virtual const Sensor* sensorAt(std::size_t index) const noexcept = 0;

    //----------[ CONFIGURATION ]----------//

    /// @brief Applies provider-specific persisted configuration.
    /// @param config The opaque provider configuration object.
    /// @return True when configuration was applied; false leaves this provider unchanged.
    [[nodiscard]] virtual bool loadConfiguration(json::Reader config) {
        (void)config;
        return true;
    }

    /// @brief Writes provider-specific persisted configuration as one JSON object.
    /// @param writer Destination JSON writer.
    /// @return True when the configuration was written successfully.
    [[nodiscard]] virtual bool saveConfiguration(json::Writer& writer) const {
        return writer.writeObject([](json::ObjectWriter&) { return true; });
    }
};

} // namespace mg::sensor
