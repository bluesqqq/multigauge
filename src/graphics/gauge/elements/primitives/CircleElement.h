#pragma once

#include "graphics/gauge/Element.h"

class CircleElement : public Element {
    private:
        FillStroke color;
        
    public:
        explicit CircleElement(Element* parent);

        CircleElement(Element* parent, const rapidjson::Value::ConstObject json);

        void draw(Graphics& g) const override;
};
