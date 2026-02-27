#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

class RectangleElement : public Element {
    private:
        Paint color;
        float radius = 0.0f;

        // Editable props list
        MG_EDITABLE_BEGIN()
            MG_PROP(radius)
            MG_PROP(color)
        MG_EDITABLE_END()

    public:
        explicit RectangleElement(Element* parent);

        RectangleElement(Element* parent, const rapidjson::Value::ConstObject json);

        void draw(Graphics& g) const override;
};

REGISTER_ELEMENT_TYPE("rectangle", RectangleElement);