#pragma once

#include <vector>
#include <string>

struct Unit {
    const char* name;
    const char* abbreviation;
    float factor;
    float offset;
    uint8_t decimalPlaces;
};

#define DEFAULT_UNIT -1

class UnitType {
    private:
        std::vector<Unit> units;
        uint8_t defaultUnitIndex;

    public:
        UnitType(const char* baseName, const char* baseAbbreviation, uint8_t baseDecimalPlaces, const std::vector<Unit>& conversionUnits = {}, uint8_t defaultUnitIndex = 0);

        void setDefaultUnit(uint8_t index);

        /// @brief Converts a value from one unit to another.
        /// @param value The value in the source unit.
        /// @param fromIndex The index of the source unit.
        /// @param toIndex The index of the target unit.
        /// @return The converted value in the target unit.
        float convert(float value, int fromIndex, int toIndex) const;

        /// @brief Converts a value from a specified unit to the base unit.
        /// @param value The value in the specified unit.
        /// @param index The index of the specified unit.
        /// @return The converted value in the base unit.
        float convertToBase(float value, int index) const;

        /// @brief Converts a value from the base unit to a specified unit.
        /// @param value The value in the base unit.
        /// @param index The index of the target unit.
        /// @return The converted value in the target unit.
        float convertFromBase(float value, int index) const;

        const Unit& getUnit(int index = DEFAULT_UNIT) const;

        const std::vector<Unit>& getUnits() const;

        const Unit& getBaseUnit() const;

        const Unit& getDefaultUnit() const;

        /// @brief Returns a string representation of a value for a specified unit.
        /// @param value The value in the specified unit.
        /// @param index The index of the specified unit.
        /// @param abbreviation If true, appends the unit abbreviation to the string.
        /// @return The value, in the specified unit, as a string.
        std::string getValueString(float value, int index = DEFAULT_UNIT, bool abbreviation = true) const;
        
        /// @brief Returns a list of unit names or abbreviations.
        /// @param abbreviations If true, returns the unit abbreviations; if false, returns the unit names/
        /// @return A vector of pointers to the unit strings (const char*).
        std::vector<const char*> listUnitStrings(bool abbreviations = false) const;

        int getIndexFromName(const char* name);

        int getIndexFromAbbreviation(const char* abbreviation);
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
//extern UnitType time;