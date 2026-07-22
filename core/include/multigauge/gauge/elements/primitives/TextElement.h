#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>
#include <multigauge/text/EmbeddedText.h>

namespace mg::gauge {

using ::mg::graphics::TextPaint;

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
    MG_PROP(text, "text", "Text", "Text to display.")
    MG_PROP(paint, "paint", "Paint", "Paint options for the text.")
    MG_PROP(useEllipses, "ellipses", "Ellipses", "Use ellipses for overflowing text.")
    MG_PROP(useHyphens, "hyphens", "Hyphens", "Use hyphens for text justification.")
        MG_PROPS_END()

    public:
        using Element::Element;
        
        void draw(Graphics& g) const override;
};

} // namespace mg::gauge
