#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>

namespace mg::gauge {
void CircularElement::resolveInherited(::mg::ValueView value,
                                       float startAngle,
                                       float endAngle) noexcept {
    resolvedValue_ = value_.value_or(value);
    resolvedStartAngle_ = startAngle_.value_or(startAngle);
    resolvedEndAngle_ = endAngle_.value_or(endAngle);
}

::mg::ValueView CircularElement::resolvedValueView() const { return resolvedValue_; }

float CircularElement::resolvedStartAngle() const { return resolvedStartAngle_; }

float CircularElement::resolvedEndAngle() const { return resolvedEndAngle_; }
} // namespace mg::gauge
