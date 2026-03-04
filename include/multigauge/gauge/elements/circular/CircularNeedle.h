#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>

class CircularNeedle : public CircularElement {
    MG_EDITOR_NAME("Circular Needle")
    private:
        Paint paint;
        float radius = 1.0f;

        MG_EDITOR_BEGIN()
            MG_EDITOR_PROP(value, "value", "Value", "Value to display. Make null to inherit from parent.")
            MG_EDITOR_PROP(startAngle, "startAngle", "Start Angle", "Angle to start from. Make null to inherit from parent.")
            MG_EDITOR_PROP(endAngle, "endAngle", "End Angle", "Angle to end at. Make null to inherit from parent.")
            MG_EDITOR_PROP(paint, "paint", "Paint", "Paint options for the needle.")
            MG_EDITOR_PROP(radius, "radius", "Radius", "Radius of the needle.")
        MG_EDITOR_END()
        
    public:
        using CircularElement::CircularElement;

        void draw(Graphics& g) const override;
};