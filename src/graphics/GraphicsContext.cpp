#include "GraphicsContext.h"

int GraphicsContext::getScreenWidth() const { return width; }

int GraphicsContext::getScreenHeight() const { return height; }

float GraphicsContext::getTextWidth(std::string_view text, std::string family, float pt, FontWeight weight, FontSlant slant) {
    if (text.empty()) return 0;

    std::string tmp(text);
    return getTextWidth(tmp.c_str(), family, pt, weight, slant);
}
