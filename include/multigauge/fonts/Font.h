#pragma once

#include <string>

namespace mg {

enum class FontWeight { Normal, Bold };
enum class FontSlant { Normal, Italic };

struct FontSpec {
    std::string family;
    float pxSize = 16.0f;
    FontWeight weight = FontWeight::Normal;
    FontSlant slant = FontSlant::Normal;
};

}

using mg::FontSlant;
using mg::FontSpec;
using mg::FontWeight;
