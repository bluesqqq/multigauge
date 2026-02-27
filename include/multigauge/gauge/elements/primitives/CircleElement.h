#pragma once

#include <multigauge/gauge/Element.h>

class CircleElement : public Element {
    private:
        Paint color;
        
    public:
        explicit CircleElement(Element* parent);

        CircleElement(Element* parent, const rapidjson::Value::ConstObject json);

        void draw(Graphics& g) const override;
};

REGISTER_ELEMENT_TYPE("circle", CircleElement);