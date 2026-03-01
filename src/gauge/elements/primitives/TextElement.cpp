#include <multigauge/gauge/elements/primitives/TextElement.h>

void TextElement::draw(Graphics &g) const {
    const auto& b = getBounds();

    if (paint.color) { 
        g.setTextPaint(paint);

        if (text.find('{') != std::string::npos) {
            const std::string expanded = embed_render::replaceEmbeds(text);
            g.drawTextArea(expanded, b.toInt(), anchor, useEllipses, useHyphens);
        } else {
            g.drawTextArea(text, b.toInt(), anchor, useEllipses, useHyphens);
        }
    }
}