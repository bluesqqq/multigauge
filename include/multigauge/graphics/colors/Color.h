#pragma once

#include <multigauge/graphics/colors/rgba.h>
#include <multigauge/io/Log.h>
#include <multigauge/values/value.h>
#include <multigauge/json/rj_helpers.h>
#include <multigauge/editor/Editable.h>

#include <rapidjson/document.h>

#include <memory>

#define DEFAULT_COLOR { 0, 0, 0, 255}

class Color;              // forward declare
class ColorTimeline;      // forward declare

using OwnedColor = std::unique_ptr<Color>;

class Color : public Editable {
    public:
        virtual ~Color() = default;
        
        /// @brief Creates a clone of this Color object
        /// @return A unique pointer to the created Color object
        virtual OwnedColor clone() const = 0;

        /// @brief Creates a Color object from JSON.
        /// @param colorJson The JSON object containing Color data
        /// @return A unique pointer to the created Color object (defaults to StaticColor if type is missing or invalid)
        static OwnedColor fromJson(const rapidjson::Value& json);

        /// @brief Gets the current color value.
        /// @return The 16-bit color value
        virtual rgba getColor() const = 0;

        /// @brief Gets the color timeline if this color type has one.
        /// @return Pointer to ColorTimeline, or nullptr if this color has no timeline
        virtual const ColorTimeline* getTimeline() const;

        /// @brief Color type identifiers
        enum class Type { Static, Value, Time, User };

        /// @brief Gets the type of this color.
        /// @return The Type enum value
        virtual Type getType() const = 0;

        //----------[ BLENDING ]----------//
        
        /// @brief Blends this color with a static color value.
        /// @param color The 16-bit color value to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = blend color)
        /// @return A new Color object with the blended result
        virtual OwnedColor blended(rgba color, float alpha) const = 0;

        /// @brief Blends this color with another Color object.
        /// @param color The Color object to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = other color)
        /// @return A new Color object with the blended result
        virtual OwnedColor blended(const Color& color, float alpha) const = 0;
};

inline bool setColor(const rapidjson::Value::ConstObject &json, const char *key, OwnedColor& out) {
    auto it = json.FindMember(key);
    if (it == json.MemberEnd()) return false;

    const auto& v = it->value;

    if (v.IsNull()) { out.reset(); return true; }
    auto color = Color::fromJson(v);
    if (!color) {
        LOG_WARN("setColor", "Key '%s' could not be parsed as Color from %s", key, rjTypeName(v));
        return false;
    }

    out = std::move(color);
    return true;
}

template<>
struct Codec<OwnedColor> {
    static bool decode(const rapidjson::Value& v, OwnedColor& out) {
        out = Color::fromJson(v);
        return (out != nullptr);
    }
};

//----------[ FILL STROKE ]----------//

struct Paint : public Editable {
    OwnedColor fill;
    OwnedColor stroke;
    float thickness = 1.0f;

    MG_EDITABLE_BEGIN()
        MG_EDITABLE_PROP(fill)
        MG_EDITABLE_PROP(stroke)
        MG_EDITABLE_PROP(thickness)
    MG_EDITABLE_END()

    Paint();

    Paint(OwnedColor fill, OwnedColor stroke, float thickness = 1.0f);

    Paint blended(rgba color, float alpha) const;
    Paint blended(const Color& color, float alpha) const;
    Paint blended(const Paint& other, float alpha) const;
};