#include <cmath>
#include <multigauge/gauge/elements/primitives/TextElement.h>
#include <multigauge/graphics/Graphics.h>
#include <multigauge/text/EmbeddedText.h>

namespace mg::gauge {
void TextElement::draw(::mg::graphics::Graphics& g, const ::mg::Rect<float>& bounds) const {
    if (!paint_.color) return;
    g.setTextPaint(paint_);
    const auto b = bounds.toInt();
    if (text_.find('{') != std::string::npos) {
        std::string expanded;
        ::mg::text::TextBuffer buffer(expanded);
        if (::mg::text::EmbeddedText(text_).render(buffer)) {
            g.drawTextArea(expanded, b, anchor_, useEllipses_, useHyphens_);
            return;
        }
    }
    g.drawTextArea(text_, b, anchor_, useEllipses_, useHyphens_);
}
} // namespace mg::gauge
