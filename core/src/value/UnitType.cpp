#include <multigauge/value/UnitType.h>

#include <multigauge/utils/Math.h>

#include <array>

namespace mg {

/* ----- CONSTRUCTOR ----- */

UnitType::UnitType(
    std::string_view name,
    std::string_view baseName,
    std::string_view baseAbbreviation,
    std::uint8_t baseDecimalPlaces,
    std::span<const Unit> conversionUnits,
    UnitIndex defaultUnit
)
    : name(name)
{
    units.reserve(conversionUnits.size() + 1);

    units.push_back(Unit{
        .name = baseName,
        .abbreviation = baseAbbreviation,
        .factor = 1.0,
        .offset = 0.0,
        .decimalPlaces = baseDecimalPlaces
    });

    units.insert(units.end(), conversionUnits.begin(), conversionUnits.end());

    setDefaultUnit(defaultUnit);
}

/* ----- LOOKUP ----- */

const UnitType* UnitType::find(std::string_view name) {
    if (name == "temperature")   return &temperature;
    if (name == "distance")      return &distance;
    if (name == "pressure")      return &pressure;
    if (name == "velocity")      return &velocity;
    if (name == "acceleration")  return &acceleration;
    if (name == "volume")        return &volume;
    if (name == "volumePerTime") return &volumePerTime;
    if (name == "revolutions")   return &revolutions;
    if (name == "angle")         return &angle;
    if (name == "percentage")    return &percentage;

    return nullptr;
}

/* ----- CONFIGURATION ----- */

void UnitType::setDefaultUnit(UnitIndex index) {
    defaultUnit = isValidIndex(index) ? index : 0;
}

/* ----- CONVERSION ----- */

float UnitType::convert(
    float value,
    UnitIndex fromIndex,
    UnitIndex toIndex
) const {
    if (fromIndex == toIndex) return value;
    float baseValue = convertToBase(value, fromIndex);
    return convertFromBase(baseValue, toIndex);
}

float UnitType::convertToBase(
    float value,
    UnitIndex index
) const {
    const Unit& unit = getUnit(index);
    return (value - unit.offset) / unit.factor;
}

float UnitType::convertFromBase(
    float value, 
    UnitIndex index
) const {
    const Unit& unit = getUnit(index);
    return (value * unit.factor) + unit.offset;
}

/* ----- UNIT ACCESS ----- */

const Unit &UnitType::getUnit(UnitIndex index) const {
    return (isValidIndex(index))
                ? units[index]
                : getDefaultUnit();
}

const std::vector<Unit> &UnitType::getUnits() const {
    return units;
}

const Unit &UnitType::getBaseUnit() const {
    return units[0];
}

const Unit &UnitType::getDefaultUnit() const {
    return units[defaultUnit];
}

/* ----- FORMATTING ----- */

std::string UnitType::getValueString(
    float value,
    UnitIndex index,
    bool abbreviation
) const {
    const Unit& unit = getUnit(index);

    std::string result = ::mg::utils::floatToString(value, unit.decimalPlaces);

    if (abbreviation && !unit.abbreviation.empty()) {
        result.append(unit.abbreviation);
    }

    return result;
}

/* ----- PRIVATE ----- */

bool UnitType::isValidIndex(UnitIndex index) const noexcept {
    return index >= 0 && static_cast<std::size_t>(index) < units.size();
}

/* ----- UNIT TYPES ----- */

namespace {

constexpr std::array temperatureConversions{
    Unit{"fahrenheit", "F", 1.8F, 32.0F, 2},
    Unit{"kelvin", "K", 1.0F, 273.15F, 2},
};

constexpr std::array distanceConversions{
    Unit{"foot", "ft", 3.28084F, 0.0F, 2},
    Unit{"kilometer", "km", 0.001F, 0.0F, 2},
    Unit{"mile", "mi", 0.00062137F, 0.0F, 1},
};

constexpr std::array pressureConversions{
    Unit{"bar", "bar", 0.0689476F, 0.0F, 4},
    Unit{"inHg", "inHg", 2.03602F, 0.0F, 1},
    Unit{"kPa", "kPa", 6.89476F, 0.0F, 1},
};

constexpr std::array velocityConversions{
    Unit{"mph", "mph", 0.621371F, 0.0F, 1},
};

constexpr std::array accelerationConversions{
    Unit{"ft/s²", "ft/s²", 3.2808399F, 0.0F, 2},
    Unit{"g-force", "g", 0.10197162F, 0.0F, 2},
};

constexpr std::array volumeConversions{
    Unit{"gallon", "gal", 0.264172F, 0.0F, 3},
    Unit{"cubic centimeter", "cc", 1000.0F, 0.0F, 0},
};

constexpr std::array volumePerTimeConversions{
    Unit{"milliliter per minute", "mL/min", 16.6666667F, 0.0F, 1},
    Unit{"gallon per hour", "GPH", 0.264172F, 0.0F, 1},
};

} // namespace

UnitType temperature{
    "temperature",
    "celsius",
    "°C",
    2,
    temperatureConversions
};

UnitType distance{
    "distance",
    "meter",
    "m",
    2,
    distanceConversions
};

UnitType pressure{
    "pressure",
    "psi",
    "psi",
    1,
    pressureConversions
};

UnitType velocity{
    "velocity",
    "kilometer per hour",
    "km/h",
    2,
    velocityConversions
};

UnitType acceleration{
    "acceleration",
    "meter per second squared",
    "m/s²",
    2,
    accelerationConversions
};

UnitType volume{
    "volume",
    "liter",
    "L",
    2,
    volumeConversions
};

UnitType volumePerTime{
    "volumePerTime",
    "liter per hour",
    "L/h",
    2,
    volumePerTimeConversions
};

UnitType revolutions{
    "revolutions",
    "revolutions per minute",
    "rpm",
    0
};

UnitType angle{
    "angle",
    "degree",
    "°",
    0
};

UnitType percentage{
    "percentage",
    "percent",
    "%",
    1
};

}
