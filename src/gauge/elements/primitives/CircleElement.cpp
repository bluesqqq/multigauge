#include <multigauge/gauge/elements/primitives/CircleElement.h>

namespace mg::gauge {

void CircleElement::draw(Graphics &g) const {
    const auto& b = getBounds();
    
    g.setPaint(paint);
    g.drawCircleInRect(b.toInt());
}

} // namespace mg::gauge
