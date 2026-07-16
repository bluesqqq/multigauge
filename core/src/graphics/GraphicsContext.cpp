#include <multigauge/graphics/GraphicsContext.h>

namespace mg::graphics {

int GraphicsContext::width() const { return w; }

int GraphicsContext::height() const { return h; }

bool GraphicsContext::resize(int w, int h) { this->w = w; this->h = h; return true; }

float GraphicsContext::getTextWidth(std::string_view text, std::string family, float pt, FontWeight weight, FontSlant slant) {
    if (text.empty()) return 0;

    std::string tmp(text);
    return getTextWidth(tmp.c_str(), family, pt, weight, slant);
}

} // namespace mg::graphics
