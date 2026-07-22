#include <multigauge/gauge/elements/primitives/TextElement.h>

namespace mg::gauge {

void TextElement::draw(Graphics &g) const {
    const auto& b = getBounds();

    if (paint.color) {
        g.setTextPaint(paint);

        if (text.find('{') != std::string::npos) {
            std::string expanded;
            ::mg::text::TextBuffer buffer(expanded);
            const ::mg::text::EmbeddedText embedded(text);

            if (embedded.render(buffer)) {
                g.drawTextArea(expanded, b.toInt(), anchor, useEllipses, useHyphens);
                return;
            }
        }

        g.drawTextArea(text, b.toInt(), anchor, useEllipses, useHyphens);
    }
}

} // namespace mg::gauge
