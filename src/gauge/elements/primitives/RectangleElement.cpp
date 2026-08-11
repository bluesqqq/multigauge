#include <cmath>
#include <multigauge/gauge/elements/primitives/RectangleElement.h>
#include <multigauge/graphics/Graphics.h>

namespace mg::gauge {
void RectangleElement::draw(::mg::graphics::Graphics& g, const ::mg::Rect<float>& bounds) const {
    const auto b = bounds.toInt();
    g.setPaint(paint_);
    if (radius_ > 0)
        g.drawRoundedRect(b, radius_);
    else
        g.drawRect(b);
}
} // namespace mg::gauge
