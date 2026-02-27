#include <multigauge/gauge/elements/primitives/RectangleElement.h>

void RectangleElement::draw(Graphics &g) const {
    const auto& b = getBounds().toInt();

    g.setPaint(color);

    if (radius > 0.0f) g.drawRoundedRect(b, radius);
    else g.drawRect(b);
}