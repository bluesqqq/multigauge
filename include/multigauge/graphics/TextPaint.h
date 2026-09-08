#pragma once

#include <multigauge/graphics/font/Font.h>
#include <multigauge/graphics/colors/Color.h>


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
    MG_PROP(family, "family")
    MG_PROP(pt, "pt")
    MG_PROP(color, "color")
    MG_PROPS_END()

    TextPaint() = default;
};

} // namespace mg::graphics
