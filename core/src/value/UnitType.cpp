#include <multigauge/value/UnitType.h>

#include <multigauge/utils/Math.h>

#include <cstring>

namespace mg {

UnitType::UnitType(const char* baseName, const char* baseAbbreviation, uint8_t baseDecimalPlaces, const std::vector<Unit>& conversionUnits, uint8_t defaultUnitIndex) {
    units.push_back({baseName, baseAbbreviation, 1.0, 0.0, baseDecimalPlaces}); // Base unit
    units.insert(units.end(), conversionUnits.begin(), conversionUnits.end());  // All other units
    setDefaultUnit(defaultUnitIndex);
}

const UnitType& UnitType::find(const char* name) {
    if (!name) return percentage;

    if (strcmp(name, "temperature") == 0) return temperature;
    if (strcmp(name, "distance") == 0) return distance;
    if (strcmp(name, "pressure") == 0) return pressure;
    if (strcmp(name, "velocity") == 0) return velocity;
    if (strcmp(name, "acceleration") == 0) return acceleration;
    if (strcmp(name, "volume") == 0) return volume;
    if (strcmp(name, "volumePerTime") == 0) return volumePerTime;
    if (strcmp(name, "revolutions") == 0) return revolutions;
    if (strcmp(name, "angle") == 0) return angle;
    if (strcmp(name, "percentage") == 0) return percentage;

    return percentage;
}

void UnitType::setDefaultUnit(uint8_t index) { defaultUnitIndex = (index >= 0 && index < units.size()) ? index : 0; }

float UnitType::convert(float value, int fromIndex, int toIndex) const {
    if (fromIndex == toIndex) return value;
    float baseValue = convertToBase(value, fromIndex);
    return convertFromBase(baseValue, toIndex);
}

float UnitType::convertToBase(float value, int index) const {
    const Unit& unit = getUnit(index);
    return (value - unit.offset) / unit.factor;
}

float UnitType::convertFromBase(float value, int index) const {
    const Unit& unit = getUnit(index);
    return (value * unit.factor) + unit.offset;
}

const Unit &UnitType::getUnit(int index) const { return (index >= 0 && index < units.size()) ? units[index] : getDefaultUnit(); }

const std::vector<Unit> &UnitType::getUnits() const { return units;}

const Unit &UnitType::getBaseUnit() const { return units[0]; }

const Unit &UnitType::getDefaultUnit() const { return (defaultUnitIndex >= 0 && defaultUnitIndex < units.size()) ? units[defaultUnitIndex] : getBaseUnit(); }

std::string UnitType::getValueString(float value, int index, bool abbreviation) const {
    const Unit& unit = getUnit(index);
    return ::mg::utils::floatToString(value, unit.decimalPlaces) + (abbreviation ? unit.abbreviation : "");
}

std::vector<const char*> UnitType::listUnitStrings(bool abbreviations) const {
    std::vector<const char*> result;

    for (const auto& unit : units)
        result.push_back(abbreviations ? unit.abbreviation : unit.name);

    return result;
}

int UnitType::getIndexFromName(const char *name) {
    if (!name) return DEFAULT_UNIT;
    for (uint8_t i = 0; i < units.size(); ++i)
        if (strcmp(name, units[i].name) == 0) return i;
    return DEFAULT_UNIT;
}

int UnitType::getIndexFromAbbreviation(const char *abbreviation) {
    if (!abbreviation) return DEFAULT_UNIT;
    for (uint8_t i = 0; i < units.size(); ++i)
        if (strcmp(abbreviation, units[i].abbreviation) == 0) return i;
    return DEFAULT_UNIT;
}

UnitType temperature(
    "celcius", "C", 2,
    {
        {"fahrenheit", "F", 1.8, 32.0, 2},
        {"kelvin", "K", 1.0, 273.15, 2}
    }
);

UnitType distance(
    "meter", "m", 2,
    {
        {"foot", "ft", 3.28084, 0.0, 2},
        {"kilometer", "km", 0.001, 0.0, 2},
        {"mile", "mi", 0.00062137, 0.0, 1}
    }
);

UnitType pressure(
    "psi", "psi", 1,
    {
        {"bar", "bar", 0.0689476, 0.0, 4},
        {"inHg", "inHg", 2.03602, 0.0, 1},
        {"kpa", "kpa", 6.89476, 0.0, 1}
    }
);

UnitType velocity(
    "km/h", "km/h", 2,
    {
        {"mph", "mph", 0.621371, 0.0, 1}
    }
);

UnitType acceleration(
    "m/s2", "m/s2", 2,
    {
        {"ft/s2", "ft/s2", 3.2808398950131, 0.0, 2},
        {"G-Force", "g", 0.10197162129779283, 0.0, 2}
    }
);

UnitType volume(
    "liter", "l", 2,
    {
        {"gallon", "l", 0.264172, 0.0, 3},
        {"cc", "l", 1000.0, 0.0, 0}
    }
);

UnitType revolutions(
    "rpm", "rpm", 0
);

UnitType angle(
    "degrees", "", 0
);

UnitType percentage(
    "percent", "%", 1
);

UnitType volumePerTime(
    "L/h", "L/h", 2,
    {
        {"mL/min", "mL/min", 16.6666666667, 0.0, 1},
        {"GPH", "GPH", 0.264172, 0.0, 1}
    }
);

}
