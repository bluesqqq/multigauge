#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/DisplayValue.h>
#include <multigauge/gauge/ticks/TickList.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>

class CircularScale : public CircularElement {
    MG_EDITOR_NAME("Circular Scale")
    private:
        TickList ticks;

        float radius = 1.0f;

        MG_EDITOR_BEGIN()
            MG_EDITOR_PROP(value, "value", "Value", "Value to display. Make null to inherit from parent.")
            MG_EDITOR_PROP(startAngle, "startAngle", "Start Angle", "Angle to start from. Make null to inherit from parent.")
            MG_EDITOR_PROP(endAngle, "endAngle", "End Angle", "Angle to end at. Make null to inherit from parent.")
            MG_EDITOR_PROP(ticks, "ticks", "Ticks", "List of ticks to draw.")
            MG_EDITOR_PROP(radius, "radius", "Radius", "Radius of the scale.")
        MG_EDITOR_END()

    public:
        using CircularElement::CircularElement;

        void draw(Graphics& g) const override;

        void update(int deltaTime) override;
};