#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>

class CircularNeedle : public CircularElement {
        MG_EDITOR_NAME("Circular Needle")
    MG_TYPE_ID("circular-needle")
    private:
        Paint paint;
        float radius = 1.0f;

        MG_PROPS_PARENT(CircularElement)

        MG_PROPS_BEGIN()
    MG_PROP(paint, "paint", "Paint", "Paint options for the needle.")
    MG_PROP(radius, "radius", "Radius", "Radius of the needle.")
        MG_PROPS_END()
        
    public:
        using CircularElement::CircularElement;

        void draw(Graphics& g) const override;
};



