#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>
#include <multigauge/values/ValueEmbed.h>

class TextElement : public Element {
    private:
        std::string text;
        TextPaint textPaint;

        Anchor anchor = Anchor::TopLeft;
        bool useEllipses = false;
        bool useHyphens = false;

        // Editable props list
        MG_EDITABLE_BEGIN()
            MG_EDITABLE_PROP(text)
            MG_EDITABLE_PROP(textPaint)
            MG_EDITABLE_PROP(useEllipses)
            MG_EDITABLE_PROP(useHyphens)
        MG_EDITABLE_END()

    public:
        using Element::Element;
        
        void draw(Graphics& g) const override;
};