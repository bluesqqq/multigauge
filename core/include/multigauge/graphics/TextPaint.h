#pragma once

#include <multigauge/graphics/font/Font.h>
#include <multigauge/graphics/colors/Color.h>

#include <rapidjson/document.h>

#include <stdint.h>
#include <string>

namespace mg::graphics {

struct TextPaint : public ::mg::PropertyObject {
    MG_EDITOR_NAME("Text Paint")

    std::string family = "default";
    FontWeight weight = FontWeight::Normal;
    FontSlant slant = FontSlant::Normal;
    float pt = 12.0f;
    OwnedColor color;

    MG_PROPS_BEGIN()
        // TODO : WEIGHT AND SLANT
    MG_PROP(family, "family", "Family", "Font family to display.")
    MG_PROP(pt, "pt", "Point", "Point size (in pixels) to display.")
    MG_PROP(color, "color", "Color", "Color of the text.")
    MG_PROPS_END()

    TextPaint() = default;
};

} // namespace mg::graphics
