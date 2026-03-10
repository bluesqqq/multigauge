#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

class RectangleElement : public Element {
    MG_EDITOR_NAME("Rectangle")
    private:
        Paint paint;
        float radius = 0.0f;

        // PropertyObject props list
        MG_PROPS_PARENT(Element)
        MG_PROPS_BEGIN()
            MG_PROP(paint, "paint", "Paint", "Paint options for the circle.", "group")
            MG_PROP(radius, "radius", "Radius", "Radius of the rectangle's corners.", "number")
        MG_PROPS_END()

    public:
        using Element::Element;
        
        void draw(Graphics& g) const override;
};

