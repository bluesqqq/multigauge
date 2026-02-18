#pragma once

#include "rgba.h"

#include "rapidjson/document.h"
#include <memory>
#include "values/value.h"
#include "json/rj_helpers.h"

#define DEFAULT_COLOR { 0, 0, 0, 255}

class ColorTimeline;

class Color {
    public:
        /// @brief Creates a clone of this Color object
        /// @return A unique pointer to the created Color object
        virtual std::unique_ptr<Color> clone() const = 0;

        /// @brief Creates a Color object from JSON.
        /// @param colorJson The JSON object containing Color data
        /// @return A unique pointer to the created Color object (defaults to StaticColor if type is missing or invalid)
        static std::unique_ptr<Color> fromJson(const rapidjson::Value& json);

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
        virtual std::unique_ptr<Color> blended(rgba color, float alpha) const = 0;

        /// @brief Blends this color with another Color object.
        /// @param color The Color object to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = other color)
        /// @return A new Color object with the blended result
        virtual std::unique_ptr<Color> blended(const Color& color, float alpha) const = 0;
};

inline bool setColor(const rapidjson::Value::ConstObject &json, const char *key, std::unique_ptr<Color>& out) {
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

//----------[ FILL STROKE ]----------//

struct FillStroke {
    std::unique_ptr<Color> fill;
    std::unique_ptr<Color> stroke;
    float thickness = 1.0f;

    FillStroke();

    FillStroke(std::unique_ptr<Color> fill, std::unique_ptr<Color> stroke, float thickness = 1.0f);
    FillStroke(const rapidjson::Value::ConstObject json);

    FillStroke blended(rgba color, float alpha) const;
    FillStroke blended(const Color& color, float alpha) const;
    FillStroke blended(const FillStroke& other, float alpha) const;
};