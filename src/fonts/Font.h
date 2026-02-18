#pragma once

#include <string>

enum class FontWeight { Normal, Bold };
enum class FontSlant { Normal, Italic };

struct FontSpec {
    std::string family;
    float pxSize = 16.0f;
    FontWeight weight = FontWeight::Normal;
    FontSlant slant = FontSlant::Normal;
};