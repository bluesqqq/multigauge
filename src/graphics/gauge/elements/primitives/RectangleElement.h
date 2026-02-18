#pragma once

#include "graphics/gauge/Element.h"
#include "graphics/colors/Color.h"

class RectangleElement : public Element {
    private:
        FillStroke color;

        float radius = 0.0f;

    public:
        explicit RectangleElement(Element* parent);

        RectangleElement(Element* parent, const rapidjson::Value::ConstObject json);

        void draw(Graphics& g) const override;
};
