#pragma once

#include <cstdint>

#include <multigauge/properties/Codec.h>
#include <multigauge/properties/EnumTraits.h>
#include <multigauge/properties/PropertyObject.h>

namespace mg::gauge {

/// @brief Defines the sizing behavior for one layout axis.
enum class LayoutSizeMode : std::uint8_t { Fit, Grow, Fixed, Percent };

/// @brief Defines the direction in which child elements are laid out.
enum class LayoutDirection : std::uint8_t { LeftToRight, TopToBottom };

/// @brief Stores Clay sizing options for one layout axis.
struct LayoutSize : ::mg::PropertyObject {
    LayoutSizeMode mode = LayoutSizeMode::Grow;
    float value = 0.0f;
    float limit = 0.0f;

    MG_PROPS_BEGIN()
    MG_PROP(mode, "mode", "Mode", "Clay sizing mode.")
    MG_PROP(value, "value", "Value", "Fixed size, percent, or minimum size.")
    MG_PROPS_END()
};

/// @brief Stores Clay-native layout state for an element or face.
struct Layout : ::mg::PropertyObject {
    LayoutSize width;
    LayoutSize height;
    LayoutDirection direction = LayoutDirection::TopToBottom;
    int paddingLeft = 0;
    int paddingRight = 0;
    int paddingTop = 0;
    int paddingBottom = 0;
    int childGap = 0;

    MG_PROPS_BEGIN()
    MG_PROP(width, "width", "Width", "Clay width sizing.")
    MG_PROP(height, "height", "Height", "Clay height sizing.")
    MG_PROP(direction, "direction", "Direction", "Child layout direction.")
    MG_PROP(paddingLeft, "paddingLeft", "Left Padding", "Left child padding.")
    MG_PROP(paddingRight, "paddingRight", "Right Padding", "Right child padding.")
    MG_PROP(paddingTop, "paddingTop", "Top Padding", "Top child padding.")
    MG_PROP(paddingBottom, "paddingBottom", "Bottom Padding", "Bottom child padding.")
    MG_PROP(childGap, "childGap", "Child Gap", "Space between children.")
    MG_PROPS_END()
};

} // namespace mg::gauge

namespace mg {

template <> struct EnumTraits<gauge::LayoutSizeMode> {
    static constexpr EnumOption<gauge::LayoutSizeMode> options[] = {
        {gauge::LayoutSizeMode::Fit, "fit", "Fit"},
        {gauge::LayoutSizeMode::Grow, "grow", "Grow"},
        {gauge::LayoutSizeMode::Fixed, "fixed", "Fixed"},
        {gauge::LayoutSizeMode::Percent, "percent", "Percent"},
    };
};

template <> struct EnumTraits<gauge::LayoutDirection> {
    static constexpr EnumOption<gauge::LayoutDirection> options[] = {
        {gauge::LayoutDirection::LeftToRight, "left-to-right", "Left to Right"},
        {gauge::LayoutDirection::TopToBottom, "top-to-bottom", "Top to Bottom"},
    };
};

CODEC_BEGIN(gauge::LayoutSizeMode)

DECODE() {
    return decodeEnum(v, out);
}

ENCODE() {
    return encodeEnum(out, v);
}
CODEC_END()

CODEC_BEGIN(gauge::LayoutDirection)

DECODE() {
    return decodeEnum(v, out);
}

ENCODE() {
    return encodeEnum(out, v);
}
CODEC_END()

} // namespace mg
