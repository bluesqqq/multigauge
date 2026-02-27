#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>

class CircularNeedle : public CircularElement {
    private:
        Paint color;
        float radius = 1.0f;

        MG_EDITABLE_BEGIN()
            MG_PROP(color)
            MG_PROP(radius)
        MG_EDITABLE_END()
        
    public:
        using CircularElement::CircularElement;

        void draw(Graphics& g) const override;
};