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
            MG_PROP(text)
            MG_PROP(useEllipses)
            MG_PROP(useHyphens)
        MG_EDITABLE_END()

    public:
        explicit TextElement(Element* parent, std::string text);

        TextElement(Element* parent, const rapidjson::Value::ConstObject json);
        
        void draw(Graphics& g) const override;
};

REGISTER_ELEMENT_TYPE("text", TextElement);