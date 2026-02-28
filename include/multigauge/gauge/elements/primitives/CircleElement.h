#pragma once

#include <multigauge/gauge/Element.h>

class CircleElement : public Element {
    MG_EDITOR_NAME("Circle")
    private:
        Paint color;

        MG_EDITABLE_BEGIN()
            MG_EDITABLE_PROP(color)
        MG_EDITABLE_END()
        
    public:
        using Element::Element;
        
        void draw(Graphics& g) const override;
};