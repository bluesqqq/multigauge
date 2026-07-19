#include <multigauge/value/ValueView.h>

#include <algorithm>

namespace mg {

UnitIndex ValueView::getUnitIndex() const { return unitIndex_.has_value() ? unitIndex_.value() : BASE_UNIT; }

ValueView::ValueView() {}

ValueView::ValueView(std::string_view id) : value_(id) {}

Measurement ValueView::valueBase() const {
    if (!value_) return 0.0f;
    Measurement result = value_->valueBase();
    if (minimum_.has_value()) result = std::max(minimum_.value(), result);
    if (maximum_.has_value()) result = std::min(maximum_.value(), result);
    return result;
}

Measurement ValueView::value() const {
    if (!value_) return 0.0f;
    return value_->value(getUnitIndex());
}

Measurement ValueView::interpolatedValue() const {
    if (!value_) return 0.5f;
    Measurement minimum = minimumBase();
    Measurement maximum = maximumBase();

    if (minimum == maximum) return 0.5f;

    return (value_->valueBase() - minimum) / (maximum - minimum);
}

Measurement ValueView::minimumBase() const {
    if (!value_) return 0.0f;
    return minimum_.has_value() ? minimum_.value() : value_->minimumBase();
}

Measurement ValueView::maximumBase() const {
    if (!value_) return 1.0f;
    return maximum_.has_value() ? maximum_.value() : value_->maximumBase();
}

Measurement ValueView::minimum() const {
    if (!value_) return 0.0f;
    auto& unitType = value_->unitType();
    return unitType.convertFromBase(minimumBase(), getUnitIndex());
}

Measurement ValueView::maximum() const {
    if (!value_) return 1.0f;
    auto& unitType = value_->unitType();
    return unitType.convertFromBase(maximumBase(), getUnitIndex());
}

const Unit* ValueView::unit() const {
    if (!value_) return nullptr;

    auto& unitType = value_->unitType();

    if (const Unit* selectedUnit = unitType.unit(getUnitIndex())) {
        return selectedUnit;
    }

    return &unitType.baseUnit();
}

std::string ValueView::formatString(bool includeAbbreviation) const {
    return value_ ? value_->valueString(getUnitIndex(), includeAbbreviation) : "n/a";
}

std::string_view ValueView::name() const {
    return value_ ? value_->name().data() : "n/a";
}

} // namespace mg
