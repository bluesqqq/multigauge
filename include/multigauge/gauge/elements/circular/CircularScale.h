#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/DisplayValue.h>
#include <multigauge/gauge/ticks/TickList.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>

class CircularScale : public CircularElement {
        MG_EDITOR_NAME("Circular Scale")
    MG_TYPE_ID("circular-scale")
    private:
        TickList ticks;

        float radius = 1.0f;

        MG_PROPS_PARENT(CircularElement)

        MG_PROPS_BEGIN()
            MG_PROP(ticks, "ticks", "Ticks", "List of ticks to draw.", "group", "Ticks", "Structure")
            MG_PROP(radius, "radius", "Radius", "Radius of the scale.", "number", "Geometry", "Dimensions")
        MG_PROPS_END()

    public:
        using CircularElement::CircularElement;

        void draw(Graphics& g) const override;

        void update(int deltaTime) override;
};



