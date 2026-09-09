#include <algorithm>
#include <cmath>

#include <multigauge/constants.h>
#include <multigauge/gauge/elements/Radial.h>
#include <multigauge/graphics/Graphics.h>
#include <multigauge/utils/Math.h>

namespace mg::gauge {

void Radial::draw(::mg::graphics::Graphics& graphics, const ::mg::Rect<float>& bounds) const {
    const float radius = std::min(bounds.width, bounds.height) * 0.5F;
    const ::mg::Point<float> center{bounds.x + bounds.width * 0.5F, bounds.y + bounds.height * 0.5F};
    const ::mg::ValueView value = value_.value_or(::mg::ValueView{});

    for (const RadialPart& part : parts_) {
        const float partRadius = radius * part.radius;
        if (part.type == RadialPartType::Scale) {
            part.ticks.drawCircular(
                graphics,
                center,
                partRadius,
                startAngle_,
                endAngle_,
                value.minimumBase(),
                value.maximumBase()
            );
            continue;
        }

        const float angle = ::mg::utils::mapf(
            value.interpolationValue(),
            0.0F,
            1.0F,
            startAngle_ * DEG2RAD,
            endAngle_ * DEG2RAD
        );
        graphics.setPaint(part.paint);
        graphics.drawLine(
            ::mg::Line<float>(center.x,
                center.y,
                center.x + std::cos(angle) * partRadius,
                center.y + std::sin(angle) * partRadius
            ).toInt(),
            10
        );
    }
}

void Radial::update(std::chrono::microseconds) {
    const ::mg::ValueView value = value_.value_or(::mg::ValueView{});
    for (RadialPart& part : parts_) {
        if (part.type == RadialPartType::Scale) part.ticks.setValueView(value.valueBase());
    }
}

} // namespace mg::gauge
