#pragma once

#include <multigauge/graphics/colors/rgba.h>
#include <multigauge/io/Log.h>
#include <multigauge/values/Value.h>
#include <multigauge/properties/PropertyObject.h>
#include <multigauge/io/Log.h>

#include <rapidjson/document.h>

#include <memory>

#define DEFAULT_COLOR { 0, 0, 0, 255}

class Color;              // forward declare
class ColorTimeline;      // forward declare

using OwnedColor = std::unique_ptr<Color>;

template <>
struct MgPropWidgetTraits<OwnedColor> { static constexpr const char* value = "color"; };

template <>
struct MgPropNullableTraits<OwnedColor> { static constexpr bool value = true; };

class Color : public PropertyObject {
    MG_EDITOR_NAME("Color")
    public:
        virtual ~Color() = default;

        MG_POLYMORPHIC_REGISTRY(OwnedColor)
        static OwnedColor createByType(const char* type);
        
        /// @brief Creates a clone of this Color object
        /// @return A unique pointer to the created Color object
        virtual OwnedColor clone() const = 0;

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

template <>
struct MgPolymorphicRegistryTraits<OwnedColor> {
    static constexpr bool supported = true;

    static rapidjson::Value getTypesMeta(rapidjson::Document::AllocatorType& a) {
        return Color::registry().getTypesMeta(a);
    }
};

CODEC_BEGIN(OwnedColor)
    DECODE();

    ENCODE();
CODEC_END()

//----------[ FILL STROKE ]----------//

struct Paint : public PropertyObject {
    MG_EDITOR_NAME("Paint")
    
    OwnedColor fill;
    OwnedColor stroke;
    float thickness = 1.0f;

    MG_PROPS_BEGIN()
    MG_PROP(fill, "fill", "Fill", "Fill color.")
    MG_PROP(stroke, "stroke", "Stroke", "Stroke color.")
    MG_PROP(thickness, "thickness", "Thickness", "Thickness of the stroke.")
    MG_PROPS_END()

    Paint();

    Paint(OwnedColor fill, OwnedColor stroke, float thickness = 1.0f);

    Paint blended(rgba color, float alpha) const;
    Paint blended(const Color& color, float alpha) const;
    Paint blended(const Paint& other, float alpha) const;
};
