#include <algorithm>
#include <cmath>

#include <multigauge/constants.h>
#include <multigauge/gauge/elements/Radial.h>
#include <multigauge/graphics/Graphics.h>
#include <multigauge/utils/Math.h>

namespace mg::gauge {
namespace {
template <typename T> RadialPart::OwnedPart createPart() { return std::make_unique<T>(); }
constexpr MgPolymorphicTypeDescriptor<RadialPart::OwnedPart> partTypes[] = {
    makePolymorphicTypeDescriptor<NeedlePart, RadialPart::OwnedPart>(&createPart<NeedlePart>),
    makePolymorphicTypeDescriptor<ScalePart, RadialPart::OwnedPart>(&createPart<ScalePart>),
};
} // namespace

const RadialPart::Registry& RadialPart::registry() {
    static const Registry registry(partTypes, nullptr);
    return registry;
}

void NeedlePart::draw(::mg::graphics::Graphics& graphics, ::mg::Point<float> center, float radius,
                      const ::mg::ValueView& value, float startAngle, float endAngle) const {
    const float angle = ::mg::utils::mapf(value.interpolationValue(), 0.0F, 1.0F,
                                           startAngle * DEG2RAD, endAngle * DEG2RAD);
    graphics.setPaint(paint_);
    graphics.drawLine(::mg::Line<float>(center.x, center.y,
                                        center.x + std::cos(angle) * radius * radius_,
                                        center.y + std::sin(angle) * radius * radius_).toInt(), 10);
}

void ScalePart::draw(::mg::graphics::Graphics& graphics, ::mg::Point<float> center, float radius,
                     const ::mg::ValueView& value, float startAngle, float endAngle) const {
    ticks_.drawCircular(graphics, center, radius * radius_, startAngle, endAngle,
                        value.minimumBase(), value.maximumBase());
}
void ScalePart::update(const ::mg::ValueView& value) { ticks_.setValueView(value.valueBase()); }

void Radial::draw(::mg::graphics::Graphics& graphics, const ::mg::Rect<float>& bounds) const {
    const ::mg::ValueView value = value_.value_or(::mg::ValueView{});
    const float radius = std::min(bounds.width, bounds.height) * 0.5F;
    const ::mg::Point<float> center{bounds.x + bounds.width * 0.5F, bounds.y + bounds.height * 0.5F};
    for (const auto& part : parts_) if (part) part->draw(graphics, center, radius, value, startAngle_, endAngle_);
}
void Radial::update(std::chrono::microseconds) {
    const ::mg::ValueView value = value_.value_or(::mg::ValueView{});
    for (const auto& part : parts_) if (part) part->update(value);
}
} // namespace mg::gauge

namespace mg {
DECODE_IMPL(gauge::RadialPart::OwnedPart) { return decodePolymorphicOwned<gauge::RadialPart>(v, out); }
ENCODE_IMPL(gauge::RadialPart::OwnedPart) { return encodePolymorphicOwned(out, v); }
} // namespace mg
