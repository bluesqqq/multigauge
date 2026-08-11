#include <cmath>
#include <multigauge/gauge/elements/primitives/CircleElement.h>
#include <multigauge/graphics/Graphics.h>

namespace mg::gauge {
void CircleElement::draw(::mg::graphics::Graphics& g, const ::mg::Rect<float>& bounds) const {
    g.setPaint(paint_);
    g.drawCircleInRect(bounds.toInt());
}
} // namespace mg::gauge
