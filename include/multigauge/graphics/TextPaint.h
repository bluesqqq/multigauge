#pragma once

#include <multigauge/graphics/colors/Color.h>

#include <rapidjson/document.h>

#include <stdint.h>
#include <string>

enum class FontWeight : uint16_t { Normal = 400, Bold = 700 };
enum class FontSlant  : uint8_t { Normal, Italic };

struct TextPaint : public Editable {
    MG_EDITOR_NAME("Text Paint")

    std::string family = "default";
    FontWeight weight = FontWeight::Normal;
    FontSlant slant = FontSlant::Normal;
    float pt = 12.0f;
    OwnedColor color;

    MG_EDITABLE_BEGIN()
        // TODO : WEIGHT AND SLANT
        MG_EDITABLE_PROP_META(family, "family", "Family", "Font family to display.")
        MG_EDITABLE_PROP_META(pt, "pt", "Point", "Point size (in pixels) to display.")
        MG_EDITABLE_PROP_META(color, "color", "Color", "Color of the text.")
    MG_EDITABLE_END()

    TextPaint() = default;
};