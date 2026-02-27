#include <multigauge/gauge/elements/circular/CircularScale.h>

void CircularScale::draw(Graphics &g) const {
    const auto& b = getBounds();

    const float w = b.width;
    const float h = b.height;
    const float diameter = std::min(w, h);
    const float radius   = diameter * 0.5f * this->radius;

    const float cx = b.x + w * 0.5f;
    const float cy = b.y + h * 0.5f;

    DisplayValue value = resolvedDisplayValue();
    float startAngle = resolvedStartAngle();
    float endAngle = resolvedEndAngle();
    
    ticks.drawCircular(g, {cx, cy}, radius, startAngle, endAngle, value.getMinimumBase(), value.getMaximumBase());
}

void CircularScale::update(int deltaTime) {
    DisplayValue value = resolvedDisplayValue();

    ticks.setDisplayValue(value.getValueBase());
}