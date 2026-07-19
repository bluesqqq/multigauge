#pragma once

#include <multigauge/value/Unit.h>

#include <cstddef>
#include <span>
#include <string>
#include <string_view>

namespace mg {

/// @brief Immutable collection of units for a measurement category.
/// @details A `UnitType` owns no unit data. It references a caller-provided static unit
/// table where index 0 is the base unit used for storage and conversion.
/// Runtime user preferences, such as which unit should be displayed by default,
/// should be stored outside this class.
class UnitType {
public:
    
    /* ----- CONSTRUCTOR ----- */

    /// @brief Constructs a unit type backed by an immutable unit table.
    /// @param name Stable name identifying the unit type.
    /// @param units Units for this type. Index 0 is the base unit.
    /// @pre `units` must not be empty.
    /// @pre `units[0]` must be the base unit with factor 1 and offset 0.
    /// @note The unit table must outlive this UnitType.
    UnitType(
        std::string_view name,
        std::span<const Unit> units
    ) noexcept;

    /* ----- LOOKUP ----- */

    /// @brief Finds a registered unit type by name.
    /// @param name Name of the unit type to find.
    /// @return The matching unit type.
    [[nodiscard]]
    static const UnitType* find(std::string_view name) noexcept;

    /* ----- CONVERSION ----- */

    /// @brief Converts a value from one unit to another.
    /// @param value The value in the source unit.
    /// @param fromIndex The index of the source unit.
    /// @param toIndex The index of the target unit.
    /// @return The converted value in the target unit.
    [[nodiscard]]
    Measurement convert(
        Measurement value,
        UnitIndex fromIndex,
        UnitIndex toIndex
    ) const noexcept;

    /// @brief Converts a value from a specified unit to the base unit.
    /// @param value The value in the specified unit.
    /// @param index The index of the specified unit.
    /// @return The converted value in the base unit.
    [[nodiscard]]
    Measurement convertToBase(
        Measurement value,
        UnitIndex index
    ) const noexcept;

    /// @brief Converts a value from the base unit to a specified unit.
    /// @param value The value in the base unit.
    /// @param index The index of the target unit.
    /// @return The converted value in the target unit.
    [[nodiscard]]
    Measurement convertFromBase(
        Measurement value,
        UnitIndex index
    ) const noexcept;

    /* ----- UNIT ACCESS ----- */

    /// @brief Returns the stable name of this unit type.
    /// @return The unit type name.
    [[nodiscard]]
    std::string_view name() const noexcept;

    /// @brief Returns a unit by index.
    /// @param index Index of the unit.
    /// @return The requested unit, or the base unit when the index is invalid.
    [[nodiscard]]
    const Unit& unit(UnitIndex index) const noexcept;

    /// @brief Returns all units belonging to this unit type.
    /// @return A read-only view of the unit table.
    [[nodiscard]]
    std::span<const Unit> units() const noexcept;

    /// @brief Returns the base unit.
    /// @return The unit stored at index 0.
    [[nodiscard]]
    const Unit& baseUnit() const noexcept;

    /* ----- FORMATTING ----- */

    /// TODO: Add a non-heap-allocation version of the below function.

    /// @brief Returns a string representation of a value for a specified unit.
    /// @param value The value in the specified unit.
    /// @param index The index of the specified unit.
    /// @param includeAbbreviation If true, appends the unit abbreviation to the string.
    /// @return The value, in the specified unit, as a string.
    [[nodiscard]]
    std::string formatValue(
        Measurement value,
        UnitIndex index,
        bool includeAbbreviation = true
    ) const;

private:

    [[nodiscard]]
    static constexpr bool isValidIndex(UnitIndex index, std::size_t unitCount) noexcept {
        return index >= 0 && static_cast<std::size_t>(index) < unitCount;
    }

    std::string_view name_;          ///< Stable name identifying the unit type.
    std::span<const Unit> units_;    ///< Non-owning view of the unit table.

};

extern const UnitType temperature;
extern const UnitType distance;
extern const UnitType pressure;
extern const UnitType velocity;
extern const UnitType acceleration;
extern const UnitType volume;
extern const UnitType volumePerTime;
extern const UnitType revolutions;
extern const UnitType angle;
extern const UnitType percentage;

} // namespace mg
