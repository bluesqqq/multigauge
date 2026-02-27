#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/DisplayValue.h>
#include <multigauge/gauge/ticks/TickList.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>

class CircularScale : public CircularElement {
    private:
        TickList ticks;

        float radius = 1.0f;

        // Editable props list
        MG_EDITABLE_BEGIN()
            MG_PROP(ticks)
            MG_PROP(radius)
        MG_EDITABLE_END()

    public:
        using CircularElement::CircularElement;

        explicit CircularScale(Element* parent);

        void draw(Graphics& g) const override;

        void update(int deltaTime) override;
};