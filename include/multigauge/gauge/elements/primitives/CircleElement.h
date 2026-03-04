#pragma once

#include <multigauge/gauge/Element.h>

class CircleElement : public Element {
    MG_EDITOR_NAME("Circle")
    private:
        Paint paint;

        MG_EDITOR_BEGIN()
            MG_EDITOR_PROP(paint, "paint", "Paint", "Paint options for the circle.")
        MG_EDITOR_END()
        
    public:
        using Element::Element;
        
        void draw(Graphics& g) const override;
};