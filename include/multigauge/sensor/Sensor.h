#pragma once

#include <string_view>

#include <multigauge/sensor/Reading.h>

namespace mg::sensor {

/// @brief Read-only view of a logical sensor.
class Sensor {
public:
    /// @brief Destroys the sensor view.
    virtual ~Sensor() = default;

    //----------[ IDENTITY ]----------//

    /// @brief Returns the stable identifier used by registries and editors.
    /// @return A view valid for the lifetime of this sensor.
    [[nodiscard]]
    virtual std::string_view id() const noexcept = 0;

    /// @brief Returns the human-readable display name.
    /// @return A view valid for the lifetime of this sensor.
    [[nodiscard]]
    virtual std::string_view name() const noexcept = 0;

    //----------[ UNIT AND RANGE ]----------//

    /// @brief Returns the native output unit.
    /// @return The unit type, or nullptr when the sensor is unitless or unknown.
    /// @note The returned UnitType must outlive this sensor.
    [[nodiscard]]
    virtual const UnitType* unit() const noexcept = 0;

    /// @brief Returns the unit index used by cached readings.
    /// @return The native unit index; defaults to the UnitType base unit.
    [[nodiscard]]
    virtual UnitIndex unitIndex() const noexcept { return 0; }

    /// @brief Returns the native measurement bounds.
    /// @return A range whose unknown limits have empty optionals.
    [[nodiscard]]
    virtual Range nativeRange() const noexcept = 0;

    //----------[ READING ]----------//
    
    /// @brief Returns the cached reading from the provider's last update.
    /// @return The current cached reading.
    [[nodiscard]]
    virtual Reading reading() const noexcept = 0;
};

} // namespace mg::sensor
