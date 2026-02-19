#pragma once

#include <multigauge/graphics/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>
#include <multigauge/values/ValueEmbed.h>

class TextElement : public Element {
    private:
        std::string text;
        TextPaint textPaint;

        Anchor anchor = Anchor::TopLeft;
        bool useEllipses = false;
        bool useHyphens = false;

    public:
        explicit TextElement(Element* parent, std::string text);

        TextElement(Element* parent, const rapidjson::Value::ConstObject json);
        
        void draw(Graphics& g) const override;
};
