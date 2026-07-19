#include <multigauge/graphics/DisplayValue.h>

namespace mg::graphics {

int DisplayValue::getUnitIndex() const { return unitIndex.has_value() ? unitIndex.value() : BASE_UNIT; }

DisplayValue::DisplayValue() {}

DisplayValue::DisplayValue(const char *newId) : value(newId) {}

float DisplayValue::getValueBase() const {
    if (!value) return 0.0f;
    float result = value->valueBase();
    if (minimum.has_value()) result = std::max(minimum.value(), result);
    if (maximum.has_value()) result = std::min(maximum.value(), result);
    return result;
}

float DisplayValue::getValue() const {
    if (!value) return 0.0f;
    return value->value(static_cast<UnitIndex>(getUnitIndex()));
}

float DisplayValue::getInterpolationValue() const {
    if (!value) return 0.5f;
    float minimum = getMinimumBase();
    float maximum = getMaximumBase();

    if (minimum == maximum) return 0.5f;

    return (value->valueBase() - minimum) / (maximum - minimum);
}

float DisplayValue::getMinimumBase() const {
    if (!value) return 0.0f;
    return minimum.has_value() ? minimum.value() : value->minimumBase();
}

float DisplayValue::getMaximumBase() const {
    if (!value) return 1.0f;
    return maximum.has_value() ? maximum.value() : value->maximumBase(); }

float DisplayValue::getMinimum() const {
    if (!value) return 0.0f;
    auto& unitType = value->unitType();
    return unitType.convertFromBase(getMinimumBase(), getUnitIndex());
}

float DisplayValue::getMaximum() const {
    if (!value) return 1.0f;
    auto& unitType = value->unitType();
    return unitType.convertFromBase(getMaximumBase(), getUnitIndex());
}

const ::mg::Unit *DisplayValue::getUnit() const {
    if (!value) return nullptr;

    auto& unitType = value->unitType();

    if (const Unit* selectedUnit = unitType.unit(getUnitIndex())) {
        return selectedUnit;
    }

    return &unitType.baseUnit();
}

std::string DisplayValue::getValueString(bool abbreviation) const {
    return value ? value->valueString(static_cast<UnitIndex>(getUnitIndex()), abbreviation) : "n/a";
}

const char *DisplayValue::getName() const {
    return value ? value->name().data() : "n/a";
}

} // namespace mg::graphics
