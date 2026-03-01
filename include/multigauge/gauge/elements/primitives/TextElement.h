#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>
#include <multigauge/values/ValueEmbed.h>

class TextElement : public Element {
    MG_EDITOR_NAME("Text")
    private:
        std::string text;
        TextPaint paint;

        Anchor anchor = Anchor::TopLeft;
        bool useEllipses = false;
        bool useHyphens = false;

        // Editable props list
        MG_EDITABLE_BEGIN()
            MG_EDITABLE_PROP_META(text, "text", "Text", "Text to display.")
            MG_EDITABLE_PROP_META(paint, "paint", "Paint", "Paint options for the text.")
            MG_EDITABLE_PROP_META(useEllipses, "ellipses", "Ellipses", "Use ellipses for overflowing text.")
            MG_EDITABLE_PROP_META(useHyphens, "hyphens", "Hyphens", "Use hyphens for text justification.")
        MG_EDITABLE_END()

    public:
        using Element::Element;
        
        void draw(Graphics& g) const override;
};