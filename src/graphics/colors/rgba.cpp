#include "rgba.h"

rgba::rgba() : r(0), g(0), b(0), a(255) { }

rgba::rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : r(r), g(g), b(b), a(a) {}

rgba::rgba(const char *s) {
    rgba newRgba;
    if (parse_hex(s, newRgba) || parse_keyword(s, newRgba)) *this = newRgba;
}

void rgba::blend(const rgba &other, float t) {
    t = std::clamp(t, 0.0f, 1.0f);

    auto lerp_u8 = [&](uint8_t x, uint8_t y) -> uint8_t {
        float v = (1.0f - t) * float(x) + t * float(y);
        v = std::clamp(v, 0.0f, 255.0f);
        return (uint8_t)std::lround(v);
    };

    r = lerp_u8(r, other.r);
    g = lerp_u8(g, other.g);
    b = lerp_u8(b, other.b);
    a = lerp_u8(a, other.a);
}

rgba rgba::blended(const rgba &other, float t) const {
    t = std::clamp(t, 0.0f, 1.0f);

    auto lerp_u8 = [&](uint8_t x, uint8_t y) -> uint8_t {
        float v = (1.0f - t) * float(x) + t * float(y);
        v = std::clamp(v, 0.0f, 255.0f);
        return (uint8_t)std::lround(v);
    };

    return rgba{
        lerp_u8(r, other.r),
        lerp_u8(g, other.g),
        lerp_u8(b, other.b),
        lerp_u8(a, other.a),
    };
}