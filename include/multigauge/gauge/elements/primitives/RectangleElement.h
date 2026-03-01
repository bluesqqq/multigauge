#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

class RectangleElement : public Element {
    MG_EDITOR_NAME("Rectangle")
    private:
        Paint paint;
        float radius = 0.0f;

        // Editable props list
        MG_EDITABLE_BEGIN()
            MG_EDITABLE_PROP_META(paint, "paint", "Paint", "Paint options for the circle.")
            MG_EDITABLE_PROP_META(radius, "radius", "Radius", "Radius of the rectangle's corners.")
        MG_EDITABLE_END()

    public:
        using Element::Element;
        
        void draw(Graphics& g) const override;
};