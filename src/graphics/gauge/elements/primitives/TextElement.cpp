#include <multigauge/graphics/gauge/elements/primitives/TextElement.h>

TextElement::TextElement(Element* parent, std::string text) : Element(parent), text(text) {}

TextElement::TextElement(Element* parent, const rapidjson::Value::ConstObject json) : Element(parent, json) {
    if (!json.HasMember("props") || !json["props"].IsObject()) return;
    const rapidjson::Value::ConstObject props = json["props"].GetObject();

    if (props.HasMember("text") && props["text"].IsString())
        text = props["text"].GetString();

   setObj(props, "textPaint", textPaint);

    // TODO: Add anchor parsing

    setBool(props, "ellipses", useEllipses);
    setBool(props, "hyphens", useHyphens);
}

void TextElement::draw(Graphics &g) const {
    const auto& b = getBounds();

    if (textPaint.color) { 
        g.setTextPaint(textPaint);

        if (text.find('{') != std::string::npos) {
            const std::string expanded = embed_render::replaceEmbeds(text);
            g.drawTextArea(expanded, b.toInt(), anchor, useEllipses, useHyphens);
        } else {
            g.drawTextArea(text, b.toInt(), anchor, useEllipses, useHyphens);
        }
    }
}