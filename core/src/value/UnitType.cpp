#include <multigauge/value/UnitType.h>

#include <multigauge/utils/Math.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <utility>

namespace mg {

namespace {

constexpr Unit temperatureUnits[] = {
    {"celsius", "C", 1.0F, 0.0F, 2},
    {"fahrenheit", "F", 1.8F, 32.0F, 2},
    {"kelvin", "K", 1.0F, 273.15F, 2},
};

constexpr Unit distanceUnits[] = {
    {"meter", "m", 1.0F, 0.0F, 2},
    {"foot", "ft", 3.28084F, 0.0F, 2},
    {"kilometer", "km", 0.001F, 0.0F, 2},
    {"mile", "mi", 0.00062137F, 0.0F, 1},
};

constexpr Unit pressureUnits[] = {
    {"psi", "psi", 1.0F, 0.0F, 1},
    {"bar", "bar", 0.0689476F, 0.0F, 4},
    {"inHg", "inHg", 2.03602F, 0.0F, 1},
    {"kilopascal", "kPa", 6.89476F, 0.0F, 1},
};

constexpr Unit velocityUnits[] = {
    {"kilometer per hour", "km/h", 1.0F, 0.0F, 2},
    {"mile per hour", "mph", 0.621371F, 0.0F, 1},
};

constexpr Unit accelerationUnits[] = {
    {"meter per second squared", "m/s^2", 1.0F, 0.0F, 2},
    {"foot per second squared", "ft/s^2", 3.2808399F, 0.0F, 2},
    {"g-force", "g", 0.10197162F, 0.0F, 2},
};

constexpr Unit volumeUnits[] = {
    {"liter", "L", 1.0F, 0.0F, 2},
    {"gallon", "gal", 0.264172F, 0.0F, 3},
    {"cubic centimeter", "cc", 1000.0F, 0.0F, 0},
};

constexpr Unit volumePerTimeUnits[] = {
    {"liter per hour", "L/h", 1.0F, 0.0F, 2},
    {"milliliter per minute", "mL/min", 16.6666667F, 0.0F, 1},
    {"gallon per hour", "GPH", 0.264172F, 0.0F, 1},
};

constexpr Unit revolutionsUnits[] = {
    {"revolutions per minute", "rpm", 1.0F, 0.0F, 0},
};

constexpr Unit angleUnits[] = {
    {"degree", "deg", 1.0F, 0.0F, 0},
};

constexpr Unit percentageUnits[] = {
    {"percent", "%", 1.0F, 0.0F, 1},
};

constexpr Unit invalidUnit = {"", "", 1.0F, 0.0F, 0};

} // namespace

/* ----- CONSTRUCTOR ----- */

UnitType::UnitType(
    std::string_view name,
    std::span<const Unit> units
) noexcept
    : name_(name),
      units_(units) {
    assert(!units.empty());
    assert(units[0].factor == 1.0F);
    assert(units[0].offset == 0.0F);
}

/* ----- LOOKUP ----- */

const UnitType* UnitType::find(std::string_view name) noexcept {
    static const std::array registry{
        std::pair{std::string_view("temperature"), &temperature},
        std::pair{std::string_view("distance"), &distance},
        std::pair{std::string_view("pressure"), &pressure},
        std::pair{std::string_view("velocity"), &velocity},
        std::pair{std::string_view("acceleration"), &acceleration},
        std::pair{std::string_view("volume"), &volume},
        std::pair{std::string_view("volumePerTime"), &volumePerTime},
        std::pair{std::string_view("revolutions"), &revolutions},
        std::pair{std::string_view("angle"), &angle},
        std::pair{std::string_view("percentage"), &percentage},
    };

    const auto found = std::find_if(registry.begin(), registry.end(), [name](const auto& entry) {
        return entry.first == name;
    });

    return found == registry.end() ? nullptr : found->second;
}

/* ----- CONVERSION ----- */

float UnitType::convert(
    float value,
    UnitIndex fromIndex,
    UnitIndex toIndex
) const noexcept {
    if (fromIndex == toIndex) return value;

    const float baseValue = convertToBase(value, fromIndex);
    return convertFromBase(baseValue, toIndex);
}

float UnitType::convertToBase(
    float value,
    UnitIndex index
) const noexcept {
    const Unit& inUnit = unit(index);
    return inUnit.factor == 0.0F ? value : (value - inUnit.offset) / inUnit.factor;
}

float UnitType::convertFromBase(
    float value,
    UnitIndex index
) const noexcept {
    const Unit& outUnit = unit(index);
    return (value * outUnit.factor) + outUnit.offset;
}

/* ----- UNIT ACCESS ----- */

std::string_view UnitType::name() const noexcept {
    return name_;
}

const Unit& UnitType::unit(UnitIndex index) const noexcept {
    return isValidIndex(index, units_.size()) ? units_[static_cast<std::size_t>(index)] : baseUnit();
}

std::span<const Unit> UnitType::units() const noexcept {
    return units_;
}

const Unit& UnitType::baseUnit() const noexcept {
    return units_.empty() ? invalidUnit : units_[0];
}

/* ----- FORMATTING ----- */

std::string UnitType::formatValue(
    float value,
    UnitIndex index,
    bool includeAbbreviation
) const {
    const Unit& formatUnit = unit(index);
    std::string result = ::mg::utils::floatToString(value, formatUnit.decimalPlaces);

    if (includeAbbreviation && !formatUnit.abbreviation.empty()) {
        result.append(formatUnit.abbreviation.data(), formatUnit.abbreviation.size());
    }

    return result;
}

/* ----- UNIT TYPES ----- */

const UnitType temperature{"temperature", std::span<const Unit>{temperatureUnits}};
const UnitType distance{"distance", std::span<const Unit>{distanceUnits}};
const UnitType pressure{"pressure", std::span<const Unit>{pressureUnits}};
const UnitType velocity{"velocity", std::span<const Unit>{velocityUnits}};
const UnitType acceleration{"acceleration", std::span<const Unit>{accelerationUnits}};
const UnitType volume{"volume", std::span<const Unit>{volumeUnits}};
const UnitType volumePerTime{"volumePerTime", std::span<const Unit>{volumePerTimeUnits}};
const UnitType revolutions{"revolutions", std::span<const Unit>{revolutionsUnits}};
const UnitType angle{"angle", std::span<const Unit>{angleUnits}};
const UnitType percentage{"percentage", std::span<const Unit>{percentageUnits}};

} // namespace mg
