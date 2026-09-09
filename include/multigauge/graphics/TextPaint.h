#pragma once

#include <multigauge/graphics/font/Font.h>
#include <multigauge/graphics/colors/Color.h>
#include <multigauge/properties/meta/Inspector.h>


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

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Typography", {
        MG_PROPERTY("family", "Font", "text");
        MG_PROPERTY("pt", "Size", "number");
        MG_PROPERTY("color", "Color", "color");
    });
    MG_INSPECTOR_END()
#endif

public:
    TextPaint() = default;
};

} // namespace mg::graphics
