#include <multigauge/gauge/elements/primitives/CircleElement.h>

void CircleElement::draw(Graphics &g) const {
    const auto& b = getBounds();
    
    g.setPaint(paint);
    g.drawCircleInRect(b.toInt());
}