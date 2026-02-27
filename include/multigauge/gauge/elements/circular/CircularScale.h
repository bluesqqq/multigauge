#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/DisplayValue.h>
#include <multigauge/gauge/ticks/TickList.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>

class CircularScale : public CircularElement {
    private:
        TickList ticks;

        float radius = 1.0f;

        MG_EDITABLE_BEGIN()
            MG_EDITABLE_PROP(value)
            MG_EDITABLE_PROP(startAngle)
            MG_EDITABLE_PROP(endAngle)
            MG_EDITABLE_PROP(ticks)
            MG_EDITABLE_PROP(radius)
        MG_EDITABLE_END()

    public:
        using CircularElement::CircularElement;

        void draw(Graphics& g) const override;

        void update(int deltaTime) override;
};