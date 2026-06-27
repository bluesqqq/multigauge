#include <multigauge/gauge/elements/primitives/TextElement.h>

namespace mg::gauge {

void TextElement::draw(Graphics &g) const {
    const auto& b = getBounds();

    if (paint.color) { 
        g.setTextPaint(paint);

        if (text.find('{') != std::string::npos) {
            const std::string expanded = ::mg::values::embed_render::replaceEmbeds(text);
            g.drawTextArea(expanded, b.toInt(), anchor, useEllipses, useHyphens);
        } else {
            g.drawTextArea(text, b.toInt(), anchor, useEllipses, useHyphens);
        }
    }
}

} // namespace mg::gauge
