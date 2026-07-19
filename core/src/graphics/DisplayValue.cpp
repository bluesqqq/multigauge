#include <multigauge/graphics/DisplayValue.h>

namespace mg::graphics {

int DisplayValue::getUnitIndex() const { return unitIndex.has_value() ? unitIndex.value() : BASE_UNIT; }

DisplayValue::DisplayValue() {}

DisplayValue::DisplayValue(const char *newId) : value(newId) {}

float DisplayValue::getValueBase() const {
    if (!value) return 0.0f;
    float result = value->getValueBase();
    if (minimum.has_value()) result = std::max(minimum.value(), result);
    if (maximum.has_value()) result = std::min(maximum.value(), result);
    return result;
}

float DisplayValue::getValue() const {
    if (!value) return 0.0f;
    return value->getValue(getUnitIndex());
}

float DisplayValue::getInterpolationValue() const {
    if (!value) return 0.5f;
    float minimum = getMinimumBase();
    float maximum = getMaximumBase();

    if (minimum == maximum) return 0.5f;

    return (value->getValueBase() - minimum) / (maximum - minimum);
}

float DisplayValue::getMinimumBase() const {
    if (!value) return 0.0f;
    return minimum.has_value() ? minimum.value() : value->getMinimumBase();
}

float DisplayValue::getMaximumBase() const {
    if (!value) return 1.0f;
    return maximum.has_value() ? maximum.value() : value->getMaximumBase(); }

float DisplayValue::getMinimum() const {
    if (!value) return 0.0f;
    auto& unitType = value->getUnitType();
    return unitType.convertFromBase(getMinimumBase(), getUnitIndex());
}

float DisplayValue::getMaximum() const {
    if (!value) return 1.0f;
    auto& unitType = value->getUnitType();
    return unitType.convertFromBase(getMaximumBase(), getUnitIndex());
}

const ::mg::Unit *DisplayValue::getUnit() const {
    if (!value) return nullptr;

    auto& unitType = value->getUnitType();

    return &unitType.unit(getUnitIndex());
}

std::string DisplayValue::getValueString(bool abbreviation) const {
    return value ? value->getValueString(getUnitIndex(), abbreviation) : "n/a";
}

const char *DisplayValue::getName() const {
    return value ? value->getName() : "n/a";
}

} // namespace mg::graphics
