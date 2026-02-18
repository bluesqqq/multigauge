#pragma once

#include "graphics/gauge/Element.h"
#include "graphics/colors/StaticColor.h"
#include "graphics/DisplayValue.h"
#include <algorithm>

class ShiftLightElement : public Element {
private:
    DisplayValue value;
    float warningValue = 5000;

    Color* lightColor = nullptr;

    // Optional label
    const char* label = "Shift";

public:
    ShiftLightElement(Element* parent, const rapidjson::Value::ConstObject json) : Element(parent, json) { }

    void draw(Graphics& g) const override {
        const auto b = getBounds().toFloat();
        if (b.width <= 0.0f || b.height <= 0.0f) return;

        const float diameter = std::min(b.width, b.height);
        const float radius   = diameter * 0.5f;

        const float cx = b.x + b.width  * 0.5f;
        const float cy = b.y + b.height * 0.5f;

        const rgba c = lightColor->getColor();

        // "On" when value exceeds warning
        if (value.getValueBase() > warningValue) {
            g.setFill(c);
            g.fillCircle((int)cx, (int)cy, (int)radius);
        } else {
            g.setStroke(c);
            g.strokeCircle((int)cx, (int)cy, (int)radius);

            g.setFill(c);
            g.drawText("label", b.getCenter().toInt(), Anchor::Center);
        }
    }
};

REGISTER_ELEMENT_TYPE("shift-light", ShiftLightElement);