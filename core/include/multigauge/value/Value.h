#pragma once

#include <string>
#include <vector>
#include <multigauge/value/UnitType.h>
#include <unordered_map>

namespace mg {

class Value {
public:

    /* ----- CONSTRUCTOR ----- */

    Value(
        std::string_view id,
        std::string_view name,
        const UnitType& unitType,
        float minimumValue,
        float maximumValue
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

    /* ----- ASSIGNMENT ----- */

    Value& operator=(float newValue);

    operator float() const;

    /* ----- VALUE ----- */

    [[nodiscard]]
    float getValueBase() const noexcept;

    void  setValueBase(float newValue) noexcept;

    /// @brief Gets the value in the specified unit.
    /// @param index The index of the unit in the associated UnitType.
    /// @return The value converted to the specified unit.
    [[nodiscard]]
    float getValue(int index = BASE_UNIT) const;

    /// @brief Sets the value in the specified unit.
    /// @param newValue The value (in the source unit) to set.
    /// @param index The index of the source unit.
    void setValue(float newValue, int index = BASE_UNIT);

    /* ----- RANGE ----- */

    [[nodiscard]]
    float getMinimumBase() const noexcept;
    
    float getMaximumBase() const noexcept;

    /// @brief Gets the minimum value in the specified unit
    /// @param index The index of the specified unit.
    /// @return The minimum value converted to the specified unit.
    float getMinimum(int index = BASE_UNIT) const;

    /// @brief Gets the maximum value in the specified unit
    /// @param index The index of the specified unit.
    /// @return The maximum value converted to the specified unit.
    float getMaximum(int index = BASE_UNIT) const;

    /// @brief Returns the normalized position of the current value between its minimum and maximum.
    /// @return A float between 0.0 and 1.0 representing the value's relative position: 0.0 corresponds to the minimum, 1.0 corresponds to the maximum.
    float getInterpolationValue() const;

    /* ----- METADATA ----- */

    [[nodiscard]]
    std::string_view id() const;

    [[nodiscard]]
    std::string_view name() const;

    [[nodiscard]]
    const UnitType& unitType() const noexcept;

    /* ----- FORMATTING ----- */

    /// @brief Returns a string representation of the value for a specific unit.
    /// @param index The index of the unit.
    /// @param abbreviation If true, appends the unit abbreviation to the string.
    /// @return The value, in the specified unit, as a string.
    std::string getValueString(int index = BASE_UNIT, bool abbreviation = true) const;
    
    /// @brief Returns the longest string representation of the value's range (minimum or maximum) for a specific unit
    /// @param index The index of the unit.
    /// @param abbreviation If true, appends the unit abbreviation to the string.
    /// @return The longer of the minimum or maximum value, in the specified unit, as a string.
    std::string getLongestValueString(int index = BASE_UNIT, bool abbreviation = true);
    
private:
    std::string_view id_;
    std::string_view name_;

    float value;
    const float minimumValue;
    const float maximumValue;

    const UnitType& unitType_;

};

}
