#include <cmath>
#include <multigauge/constants.h>
#include <multigauge/gauge/elements/circular/CircularNeedle.h>
#include <multigauge/graphics/Graphics.h>
#include <multigauge/utils/Math.h>

namespace mg::gauge {
void CircularNeedle::draw(::mg::graphics::Graphics& g, const ::mg::Rect<float>& b) const {
    const float r = std::min(b.width, b.height) * .5f * radius_, cx = b.x + b.width * .5f,
                cy = b.y + b.height * .5f;
    const float a = ::mg::utils::mapf(resolvedValueView().interpolationValue(),
                                      0.f,
                                      1.f,
                                      resolvedStartAngle() * DEG2RAD,
                                      resolvedEndAngle() * DEG2RAD);
    g.setPaint(paint_);
    g.drawLine(::mg::Line<float>(cx, cy, cx + std::cos(a) * r, cy + std::sin(a) * r).toInt(), 10);
}
} // namespace mg::gauge
