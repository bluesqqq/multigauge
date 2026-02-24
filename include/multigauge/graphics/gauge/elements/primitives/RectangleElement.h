#pragma once

#include <multigauge/graphics/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

class RectangleElement : public Element {
    private:
        Paint color;

        float radius = 0.0f;

    public:
        explicit RectangleElement(Element* parent);

        RectangleElement(Element* parent, const rapidjson::Value::ConstObject json);

        void draw(Graphics& g) const override;
};

REGISTER_ELEMENT_TYPE("rectangle", RectangleElement);