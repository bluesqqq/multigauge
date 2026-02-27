#include <multigauge/gauge/elements/primitives/CircleElement.h>

void CircleElement::draw(Graphics &g) const {
    const auto& b = getBounds();
    
    g.setPaint(color);
    g.drawCircleInRect(b.toInt());
}