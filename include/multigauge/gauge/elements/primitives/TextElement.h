#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>
#include <multigauge/values/ValueEmbed.h>

class TextElement : public Element {
        MG_EDITOR_NAME("Text")
    MG_TYPE_ID("text")
    private:
        std::string text;
        TextPaint paint;

        Anchor anchor = Anchor::TopLeft;
        bool useEllipses = false;
        bool useHyphens = false;

        // PropertyObject props list
        MG_PROPS_PARENT(Element)
        MG_PROPS_BEGIN()
            MG_PROP(text, "text", "Text", "Text to display.", "Content", "Text")
            MG_PROP(paint, "paint", "Paint", "Paint options for the text.", "Appearance", "Typography")
            MG_PROP(useEllipses, "ellipses", "Ellipses", "Use ellipses for overflowing text.", "Content", "Wrapping")
            MG_PROP(useHyphens, "hyphens", "Hyphens", "Use hyphens for text justification.", "Content", "Wrapping")
        MG_PROPS_END()

    public:
        using Element::Element;
        
        void draw(Graphics& g) const override;
};



