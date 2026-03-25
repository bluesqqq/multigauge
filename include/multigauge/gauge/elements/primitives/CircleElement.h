#pragma once

#include <multigauge/gauge/Element.h>

class CircleElement : public Element {
        MG_EDITOR_NAME("Circle")
    MG_TYPE_ID("circle")
    private:
        Paint paint;

        MG_PROPS_PARENT(Element)

        MG_PROPS_BEGIN()
            MG_PROP(paint, "paint", "Paint", "Paint options for the circle.", "Appearance", "Fill & Stroke")
        MG_PROPS_END()
        
    public:
        using Element::Element;
        
        void draw(Graphics& g) const override;
};



