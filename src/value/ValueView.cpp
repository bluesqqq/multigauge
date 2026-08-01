#include <multigauge/value/ValueView.h>

#include <algorithm>

namespace mg {

UnitIndex ValueView::getUnitIndex() const noexcept { return unitIndex_.has_value() ? unitIndex_.value() : BASE_UNIT; }

ValueView::ValueView() noexcept {}

ValueView::ValueView(std::string_view id) : value_(id) {}

Measurement ValueView::valueBase() const noexcept {
    if (!value_) return 0.0f;
    Measurement result = value_.value();
    if (minimumBase_.has_value()) result = std::max(minimumBase_.value(), result);
    if (maximumBase_.has_value()) result = std::min(maximumBase_.value(), result);
    return result;
}

Measurement ValueView::value() const noexcept {
    if (!value_) return 0.0f;
    const UnitType* unitType = value_.unit();
    return unitType ? unitType->convertFromBase(valueBase(), getUnitIndex()) : 0.0F;
}

Measurement ValueView::interpolationValue() const noexcept {
    if (!value_) return 0.5f;
    Measurement minimum = minimumBase();
    Measurement maximum = maximumBase();

    if (minimum == maximum) return 0.5f;

    return (valueBase() - minimum) / (maximum - minimum);
}

Measurement ValueView::minimumBase() const noexcept {
    if (!value_) return 0.0f;
    return minimumBase_.has_value() ? minimumBase_.value() : value_.minimum();
}

Measurement ValueView::maximumBase() const noexcept {
    if (!value_) return 1.0f;
    return maximumBase_.has_value() ? maximumBase_.value() : value_.maximum();
}

Measurement ValueView::minimum() const noexcept {
    if (!value_) return 0.0f;
    const UnitType* unitType = value_.unit();
    return unitType ? unitType->convertFromBase(minimumBase(), getUnitIndex()) : 0.0F;
}

Measurement ValueView::maximum() const noexcept {
    if (!value_) return 1.0f;
    const UnitType* unitType = value_.unit();
    return unitType ? unitType->convertFromBase(maximumBase(), getUnitIndex()) : 1.0F;
}

const Unit* ValueView::unit() const noexcept {
    if (!value_) return nullptr;

    const UnitType* unitType = value_.unit();
    if (!unitType) return nullptr;

    if (const Unit* selectedUnit = unitType->unit(getUnitIndex())) {
        return selectedUnit;
    }

    return &unitType->baseUnit();
}

std::string ValueView::valueString(bool includeAbbreviation) const {
    const UnitType* unitType = value_.unit();
    return unitType ? unitType->formatValue(value(), getUnitIndex(), includeAbbreviation) : "n/a";
}

std::string_view ValueView::name() const noexcept {
    return value_ ? value_.name() : "n/a";
}

} // namespace mg
