#include "CircleElement.h"

CircleElement::CircleElement(Element* parent) : Element(parent) {}

CircleElement::CircleElement(Element* parent, const rapidjson::Value::ConstObject json) : Element(parent, json) {
    if (!json.HasMember("props") || !json["props"].IsObject()) return;
    const rapidjson::Value::ConstObject props = json["props"].GetObject();

    setObj(props, "color", color);
}

void CircleElement::draw(Graphics &g) const {
    const auto& b = getBounds();

    const float w = b.width;
    const float h = b.height;
    const float diameter = std::min(w, h);
    const float radius   = diameter * 0.5f;

    const float cx = b.x + w * 0.5f;
    const float cy = b.y + h * 0.5f;

    if (color.fill) {
        g.setFill(*color.fill);
        g.fillCircle(cx, cy, radius);
    }

    if (color.stroke) {
        g.setStroke(*color.stroke);
        g.setStrokeThickness(color.thickness);
        g.strokeCircle(cx, cy, radius);
    }
}

REGISTER_ELEMENT_TYPE("circle", CircleElement);