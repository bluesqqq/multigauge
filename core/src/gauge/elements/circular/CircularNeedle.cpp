#include <multigauge/gauge/elements/circular/CircularNeedle.h>

#include <multigauge/constants.h>
#include <multigauge/utils/Math.h>
#include <cmath>

namespace mg::gauge {

using ::mg::utils::mapf;

void CircularNeedle::draw(Graphics &g) const {
    const auto& b = getBounds();

    const float w = b.width;
    const float h = b.height;
    const float diameter = std::min(w, h);
    const float radius   = diameter * 0.5f * this->radius;

    const float cx = b.x + w * 0.5f;
    const float cy = b.y + h * 0.5f;

    const ValueView& value = resolvedValueView();
    float startAngle = resolvedStartAngle() * DEG2RAD;
    float endAngle = resolvedEndAngle() * DEG2RAD;

    float needleAngle = mapf(value.getInterpolationValue(), 0.0f, 1.0f, startAngle, endAngle);

    Line<float> needle(cx, cy, cx + cos(needleAngle) * radius, cy + sin(needleAngle) * radius);
    g.setPaint(paint);
    g.drawLine(needle.toInt(), 10);
}

} // namespace mg::gauge
