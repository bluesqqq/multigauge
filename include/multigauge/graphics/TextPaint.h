#pragma once

#include <multigauge/graphics/colors/Color.h>

#include <rapidjson/document.h>

#include <stdint.h>
#include <string>

enum class FontWeight : uint16_t { Normal = 400, Bold = 700 };
enum class FontSlant  : uint8_t { Normal, Italic };

struct TextPaint {
    std::string family = "default";
    FontWeight weight = FontWeight::Normal;
    FontSlant slant = FontSlant::Normal;
    float pt = 12.0f;
    OwnedColor color;

    TextPaint() = default;

    explicit TextPaint(const rapidjson::Value::ConstObject& json) {
        if (json.HasMember("family") && json["family"].IsString())
            family = json["family"].GetString();

        setFloat(json, "pt", pt);

        if (json.HasMember("weight") && json["weight"].IsString()) {
            const char* w = json["weight"].GetString();
            if (strcmp(w, "bold") == 0)      weight = FontWeight::Bold;
            else if (strcmp(w, "normal") == 0) weight = FontWeight::Normal;
        }

        if (json.HasMember("slant") && json["slant"].IsString()) {
            const char* s = json["slant"].GetString();
            if (strcmp(s, "italic") == 0)      slant = FontSlant::Italic;
            else if (strcmp(s, "normal") == 0) slant = FontSlant::Normal;
        }

        if (json.HasMember("color"))
            color = Color::fromJson(json["color"]);
    }
};