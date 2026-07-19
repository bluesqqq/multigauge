#pragma once

#include <span>
#include <string>
#include <string_view>

#include <multigauge/value/UnitType.h>

namespace mg {

/// @brief Runtime value stored in base units with fixed metadata and range.
/// @details `Value` represents a measured quantity, such as RPM, pressure, or
/// temperature. The stored value and range are always in the associated
/// `UnitType`'s base unit. Display-unit selection is intentionally not stored
/// here; callers pass a `UnitIndex` when they need converted values.
class Value {
public:

    /* ----- CONSTRUCTOR ----- */

    /// @brief Constructs a value with fixed identity, unit type, and range.
    /// @param id Stable identifier used by `find`.
    /// @param name Human-readable display name.
    /// @param unitType Unit type used for conversion and formatting.
    /// @param minimum Minimum allowed value in base units.
    /// @param maximum Maximum allowed value in base units.
    /// @note `id`, `name`, and `unitType` must outlive this Value.
    Value(
        std::string_view id,
        std::string_view name,
        const UnitType& unitType,
        Measurement minimum,
        Measurement maximum
    ) noexcept;
    
    /* ----- LOOKUP ----- */

    /// @brief Finds a registered value by identifier.
    /// @param id Identifier of the value to find.
    /// @return The matching value, or nullptr if no value is registered.
    [[nodiscard]]
    static Value* find(std::string_view id) noexcept;

    /// @brief Returns all registered values.
    /// @return The registered values.
    [[nodiscard]]
    static std::span<const Value> list() noexcept;

    /* ----- VALUE ----- */

    /// @brief Gets the value in base units.
    /// @return The value in base units.
    [[nodiscard]]
    Measurement valueBase() const noexcept;

    /// @brief Sets the value in base units.
    /// @param newValue The value (in base units) to set.
    void setValueBase(Measurement newValue) noexcept;

    /// @brief Gets the value in the specified unit.
    /// @param index The index of the unit in the associated UnitType.
    /// @return The value converted to the specified unit.
    [[nodiscard]]
    Measurement value(UnitIndex index = BASE_UNIT) const noexcept;

    /// @brief Sets the value in the specified unit.
    /// @param newValue The value (in the source unit) to set.
    /// @param index The index of the source unit.
    void setValue(Measurement newValue, UnitIndex index = BASE_UNIT) noexcept;

    /* ----- RANGE ----- */

    /// @brief Gets the minimum value in base units.
    /// @return The minimum value in base units.
    [[nodiscard]]
    Measurement minimumBase() const noexcept;
    
    /// @brief Gets the maximum value in base units.
    /// @return The maximum value in base units.
    [[nodiscard]]
    Measurement maximumBase() const noexcept;

    /// @brief Gets the minimum value in the specified unit.
    /// @param index The index of the specified unit.
    /// @return The minimum value converted to the specified unit.
    [[nodiscard]]
    Measurement minimum(UnitIndex index = BASE_UNIT) const noexcept;

    /// @brief Gets the maximum value in the specified unit.
    /// @param index The index of the specified unit.
    /// @return The maximum value converted to the specified unit.
    [[nodiscard]]
    Measurement maximum(UnitIndex index = BASE_UNIT) const noexcept;

    /// @brief Returns the normalized position of the current value between its minimum and maximum.
    /// @return A `Measurement` between 0.0 and 1.0 representing the value's relative position: 0.0 corresponds to the minimum, 1.0 corresponds to the maximum.
    [[nodiscard]]
    Measurement interpolationValue() const noexcept;

    /* ----- METADATA ----- */

    [[nodiscard]]
    std::string_view id() const noexcept;

    [[nodiscard]]
    std::string_view name() const noexcept;

    [[nodiscard]]
    const UnitType& unitType() const noexcept;

    /* ----- FORMATTING ----- */

    /// @brief Returns a string representation of the value for a specific unit.
    /// @param index The index of the unit.
    /// @param abbreviation If true, appends the unit abbreviation to the string.
    /// @return The value, in the specified unit, as a string.
    [[nodiscard]]
    std::string valueString(UnitIndex index = BASE_UNIT, bool abbreviation = true) const;
    
    /// @brief Returns the longest string representation of the value's range (minimum or maximum) for a specific unit
    /// @param index The index of the unit.
    /// @param abbreviation If true, appends the unit abbreviation to the string.
    /// @return The longer of the minimum or maximum value, in the specified unit, as a string.
    [[nodiscard]]
    std::string longestValueString(UnitIndex index = BASE_UNIT, bool abbreviation = true) const;
    
private:
    std::string_view id_;
    std::string_view name_;

    Measurement value_;
    const Measurement minimum_;
    const Measurement maximum_;

    const UnitType& unitType_;

};

}
