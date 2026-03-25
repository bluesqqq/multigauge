#pragma once

#include <multigauge/graphics/colors/Color.h>

#include <rapidjson/document.h>

#include <stdint.h>
#include <string>

enum class FontWeight : uint16_t { Normal = 400, Bold = 700 };
enum class FontSlant  : uint8_t { Normal, Italic };

struct TextPaint : public PropertyObject {
    MG_EDITOR_NAME("Text Paint")

    std::string family = "default";
    FontWeight weight = FontWeight::Normal;
    FontSlant slant = FontSlant::Normal;
    float pt = 12.0f;
    OwnedColor color;

    MG_PROPS_BEGIN()
        // TODO : WEIGHT AND SLANT
        MG_PROP(family, "family", "Family", "Font family to display.", "string", "Typography", "Font")
        MG_PROP(pt, "pt", "Point", "Point size (in pixels) to display.", "number", "Typography", "Font")
        MG_PROP(color, "color", "Color", "Color of the text.", "color", "Typography", "Color")
    MG_PROPS_END()

    TextPaint() = default;
};
