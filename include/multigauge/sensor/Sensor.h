#pragma once

#include <string_view>

#include <multigauge/sensor/Reading.h>

namespace mg::sensor {

/// Read-only view of a logical sensor.
///
/// Sensors are intentionally passive from the caller's point of view:
/// providers own polling and cache management, and consumers only read
/// metadata plus the most recent sample.
class Sensor {
public:
    virtual ~Sensor() = default;

    //----------[ IDENTITY ]----------//

    /// Stable identifier for registry/editor use.
    /// The returned view must remain valid for the lifetime of the sensor.
    [[nodiscard]]
    virtual std::string_view id() const noexcept = 0;

    /// Human-readable display name.
    /// The returned view must remain valid for the lifetime of the sensor.
    [[nodiscard]]
    virtual std::string_view name() const noexcept = 0;

    //----------[ UNIT AND RANGE ]----------//

    /// Native output unit of the sensor, if it has one.
    /// Returns nullptr when the sensor is unitless or unknown.
    /// The pointed-to UnitType must outlive the sensor.
    [[nodiscard]]
    virtual const UnitType* unit() const noexcept = 0;

    /// Index of the unit in `unit()` used by the cached reading. The default
    /// is the UnitType base unit, which is also the ValueRegistry storage unit.
    [[nodiscard]]
    virtual UnitIndex unitIndex() const noexcept { return 0; }

    /// Native bounds reported by the sensor.
    /// Unknown limits are represented by an empty optional inside the range.
    [[nodiscard]]
    virtual Range nativeRange() const noexcept = 0;

    //----------[ READING ]----------//
    
    /// Cached reading after the provider's last update cycle.
    [[nodiscard]]
    virtual Reading reading() const noexcept = 0;
};

} // namespace mg::sensor
