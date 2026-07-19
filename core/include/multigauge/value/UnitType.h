#pragma once

#include <multigauge/value/Unit.h>

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <string_view>

namespace mg {

class UnitType {
    public:
        
        /* ----- CONSTRUCTOR ----- */

        /// @brief Constructs a unit type backed by an immutable unit table.
        /// @param name Stable name identifying the unit type.
        /// @param units Units for this type. Index 0 is the base unit.
        /// @param defaultUnit Index returned when DEFAULT_UNIT or an invalid index is requested.
        /// @note The unit table must outlive this UnitType.
        constexpr UnitType(
            std::string_view name,
            std::span<const Unit> units,
            UnitIndex defaultUnit = 0
        ) noexcept
            : name(name),
              units(units),
              defaultUnit(isValidUnitIndex(defaultUnit, units.size()) ? defaultUnit : 0) {}

        /* ----- LOOKUP ----- */

        /// @brief Finds a registered unit type by name.
        /// @param name Name of the unit type to find.
        /// @return The matching unit type.
        [[nodiscard]]
        static UnitType* find(std::string_view name) noexcept;

        /* ----- CONFIGURATION ----- */

        /// @brief Sets the default unit used when DEFAULT_UNIT or an invalid index is requested.
        /// @param index Index of the new default unit. Invalid indexes fall back to the base unit.
        void setDefaultUnit(UnitIndex index) noexcept;

        /* ----- CONVERSION ----- */

        /// @brief Converts a value from one unit to another.
        /// @param value The value in the source unit.
        /// @param fromIndex The index of the source unit.
        /// @param toIndex The index of the target unit.
        /// @return The converted value in the target unit.
        [[nodiscard]]
        float convert(
            float value,
            UnitIndex fromIndex,
            UnitIndex toIndex
        ) const noexcept;

        /// @brief Converts a value from a specified unit to the base unit.
        /// @param value The value in the specified unit.
        /// @param index The index of the specified unit.
        /// @return The converted value in the base unit.
        [[nodiscard]]
        float convertToBase(
            float value,
            UnitIndex index
        ) const noexcept;

        /// @brief Converts a value from the base unit to a specified unit.
        /// @param value The value in the base unit.
        /// @param index The index of the target unit.
        /// @return The converted value in the target unit.
        [[nodiscard]]
        float convertFromBase(
            float value,
            UnitIndex index
        ) const noexcept;

        /* ----- UNIT ACCESS ----- */

        [[nodiscard]]
        std::string_view getName() const noexcept;

        /// @brief Returns a unit by index.
        /// @param index Index of the unit, or `DEFAULT_UNIT`.
        /// @return The requested unit, or the default unit when the index is invalid.
        [[nodiscard]]
        const Unit& getUnit(UnitIndex index = DEFAULT_UNIT) const noexcept;

        [[nodiscard]]
        std::span<const Unit> getUnits() const noexcept;

        [[nodiscard]]
        std::size_t getUnitCount() const noexcept;

        /// @brief Returns the base unit.
        /// @return The unit stored at index 0.
        [[nodiscard]]
        const Unit& getBaseUnit() const noexcept;

        /// @brief Returns the currently selected default unit.
        /// @return The default unit.
        [[nodiscard]]
        const Unit& getDefaultUnit() const noexcept;

        [[nodiscard]]
        UnitIndex getDefaultUnitIndex() const noexcept;

        /* ----- FORMATTING ----- */

        /// @brief Returns a string representation of a value for a specified unit.
        /// @param value The value in the specified unit.
        /// @param index The index of the specified unit.
        /// @param abbreviation If true, appends the unit abbreviation to the string.
        /// @return The value, in the specified unit, as a string.
        [[nodiscard]]
        std::string getValueString(
            float value,
            UnitIndex index = DEFAULT_UNIT,
            bool abbreviation = true
        ) const;

    private:
        [[nodiscard]]
        bool isValidIndex(UnitIndex index) const noexcept;

        [[nodiscard]]
        static constexpr bool isValidUnitIndex(UnitIndex index, std::size_t unitCount) noexcept {
            return index >= 0 && static_cast<std::size_t>(index) < unitCount;
        }

        std::string_view name;
        std::span<const Unit> units;
        UnitIndex defaultUnit = 0;

};

extern UnitType temperature;
extern UnitType distance;
extern UnitType pressure;
extern UnitType velocity;
extern UnitType acceleration;
extern UnitType volume;
extern UnitType volumePerTime;
extern UnitType revolutions;
extern UnitType angle;
extern UnitType percentage;



} // namespace mg
