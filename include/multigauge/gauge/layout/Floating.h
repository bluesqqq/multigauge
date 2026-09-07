#pragma once

#include <cstdint>

#include <multigauge/gauge/layout/Expand.h>
#include <multigauge/gauge/layout/Offset.h>
#include <multigauge/properties/Codec.h>
#include <multigauge/properties/EnumTraits.h>
#include <multigauge/properties/PropertyObject.h>

namespace mg::gauge::layout {

/// @brief Defines how an element participates in layout positioning.
enum class FloatingMode : std::uint8_t { Flow, Relative, Absolute };

/// @brief Defines an anchor point on an element used for floating layout.
enum class FloatingAnchor : std::uint8_t {
    LeftTop,
    LeftCenter,
    LeftBottom,
    CenterTop,
    Center,
    CenterBottom,
    RightTop,
    RightCenter,
    RightBottom,
};

/// @brief Stores Clay floating-layer placement for an element.
struct Floating : ::mg::PropertyObject {
    FloatingMode mode = FloatingMode::Flow;
    FloatingAnchor elementAnchor = FloatingAnchor::LeftTop;
    FloatingAnchor parentAnchor = FloatingAnchor::LeftTop;
    Offset offset;
    Expand expand;
    int zIndex = 0;

    MG_PROPS_BEGIN()
        MG_PROP(mode, "mode", "Mode", "Whether this item flows normally or floats relative to its parent or root.")
        MG_PROP(elementAnchor, "elementAnchor", "Element Anchor", "Anchor point on this floating item.")
        MG_PROP(parentAnchor, "parentAnchor", "Parent Anchor", "Anchor point on the attached element.")
        MG_PROP(offset, "offset", "Offset", "Pixel offset from the attached anchor.")
        MG_PROP(expand, "expand", "Expand", "Additional outer bounds in pixels.")
        MG_PROP(zIndex, "zIndex", "Z Index", "Floating draw order.")
    MG_PROPS_END()
};

} // namespace mg::gauge::layout

namespace mg {

template <> struct EnumTraits<gauge::layout::FloatingMode> {
    static constexpr EnumOption<gauge::layout::FloatingMode> options[] = {
        {gauge::layout::FloatingMode::Flow, "flow", "Flow"},
        {gauge::layout::FloatingMode::Relative, "relative", "Relative"},
        {gauge::layout::FloatingMode::Absolute, "absolute", "Absolute"},
    };
};

template <> struct EnumTraits<gauge::layout::FloatingAnchor> {
    static constexpr EnumOption<gauge::layout::FloatingAnchor> options[] = {
        {gauge::layout::FloatingAnchor::LeftTop, "left-top", "Left Top"},
        {gauge::layout::FloatingAnchor::LeftCenter, "left-center", "Left Center"},
        {gauge::layout::FloatingAnchor::LeftBottom, "left-bottom", "Left Bottom"},
        {gauge::layout::FloatingAnchor::CenterTop, "center-top", "Center Top"},
        {gauge::layout::FloatingAnchor::Center, "center", "Center"},
        {gauge::layout::FloatingAnchor::CenterBottom, "center-bottom", "Center Bottom"},
        {gauge::layout::FloatingAnchor::RightTop, "right-top", "Right Top"},
        {gauge::layout::FloatingAnchor::RightCenter, "right-center", "Right Center"},
        {gauge::layout::FloatingAnchor::RightBottom, "right-bottom", "Right Bottom"},
    };
};

CODEC_BEGIN(gauge::layout::FloatingMode)
    DECODE() { return decodeEnum(v, out); }

    ENCODE() { return encodeEnum(out, v); }
CODEC_END()

CODEC_BEGIN(gauge::layout::FloatingAnchor)
    DECODE() { return decodeEnum(v, out); }

    ENCODE() { return encodeEnum(out, v); }
CODEC_END()

} // namespace mg
