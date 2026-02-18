#pragma once

#include "graphics/gauge/Element.h"
#include "graphics/colors/Color.h"
#include "values/ValueEmbed.h"

class TextElement : public Element {
    private:
        std::string text;
        TextStyle textStyle;

        Anchor anchor = Anchor::TopLeft;
        bool useEllipses = false;
        bool useHyphens = false;

    public:
        explicit TextElement(Element* parent, std::string text);

        TextElement(Element* parent, const rapidjson::Value::ConstObject json);
        
        void draw(Graphics& g) const override;
};
