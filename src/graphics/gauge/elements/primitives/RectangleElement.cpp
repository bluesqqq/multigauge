#include "RectangleElement.h"

RectangleElement::RectangleElement(Element* parent) : Element(parent) {}

RectangleElement::RectangleElement(Element* parent, const rapidjson::Value::ConstObject json) : Element(parent, json) {
    if (!json.HasMember("props") || !json["props"].IsObject()) return;
    const rapidjson::Value::ConstObject props = json["props"].GetObject();

    setObj(props, "color", color);
    setFloat(props, "radius", radius);
}

void RectangleElement::draw(Graphics &g) const {
    const auto& b = getBounds().toInt();

    if (color.fill) {
        g.setFill(*color.fill);
        if (radius > 0.0f) g.fillRoundedRect(b, radius);
        else g.fillRect(b);
    }

    if (color.stroke) {
        g.setStroke(*color.stroke);
        g.setStrokeThickness(color.thickness);
        if (radius > 0.0f) g.strokeRoundedRect(b, radius);
        else g.strokeRect(b);
    }
}

REGISTER_ELEMENT_TYPE("rectangle", RectangleElement);