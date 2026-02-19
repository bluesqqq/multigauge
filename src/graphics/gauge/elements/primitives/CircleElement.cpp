#include <multigauge/graphics/gauge/elements/primitives/CircleElement.h>

CircleElement::CircleElement(Element* parent) : Element(parent) {}

CircleElement::CircleElement(Element* parent, const rapidjson::Value::ConstObject json) : Element(parent, json) {
    if (!json.HasMember("props") || !json["props"].IsObject()) return;
    const rapidjson::Value::ConstObject props = json["props"].GetObject();

    setObj(props, "color", color);
}

void CircleElement::draw(Graphics &g) const {
    const auto& b = getBounds();
    
    g.setPaint(color);
    g.drawCircleInRect(b.toInt());
}

REGISTER_ELEMENT_TYPE("circle", CircleElement);