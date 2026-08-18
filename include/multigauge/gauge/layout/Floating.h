#pragma once

#include <cstdint>

#include <multigauge/gauge/layout/Expand.h>
#include <multigauge/gauge/layout/Offset.h>
#include <multigauge/properties/Codec.h>
#include <multigauge/properties/EnumTraits.h>
#include <multigauge/properties/PropertyObject.h>

namespace mg::gauge::layout {

/// @brief Defines the element a floating layout node is attached to.
enum class FloatingAttachTo : std::uint8_t { None, Parent, Root };

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
    FloatingAttachTo attachTo = FloatingAttachTo::None;
    FloatingAnchor elementAnchor = FloatingAnchor::LeftTop;
    FloatingAnchor parentAnchor = FloatingAnchor::LeftTop;
    Offset offset;
    Expand expand;
    int zIndex = 0;
    bool fillParent = false;

    MG_PROPS_BEGIN()
        MG_PROP(attachTo, "attachTo", "Attach To", "Element this floating item is attached to.")
        MG_PROP(elementAnchor, "elementAnchor", "Element Anchor", "Anchor point on this floating item.")
        MG_PROP(parentAnchor, "parentAnchor", "Parent Anchor", "Anchor point on the attached element.")
        MG_PROP(offset, "offset", "Offset", "Pixel offset from the attached anchor.")
    MG_PROP(expand, "expand", "Expand", "Additional outer bounds in pixels.")
    MG_PROP(zIndex, "zIndex", "Z Index", "Floating draw order.")
    MG_PROP(fillParent, "fillParent", "Fill Parent", "Use the attached parent's final bounds.")
    MG_PROPS_END()
};

} // namespace mg::gauge::layout

namespace mg {

template <> struct EnumTraits<gauge::layout::FloatingAttachTo> {
    static constexpr EnumOption<gauge::layout::FloatingAttachTo> options[] = {
        {gauge::layout::FloatingAttachTo::None, "none", "None"},
        {gauge::layout::FloatingAttachTo::Parent, "parent", "Parent"},
        {gauge::layout::FloatingAttachTo::Root, "root", "Root"},
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

CODEC_BEGIN(gauge::layout::FloatingAttachTo)
    DECODE() { return decodeEnum(v, out); }

    ENCODE() { return encodeEnum(out, v); }
CODEC_END()

CODEC_BEGIN(gauge::layout::FloatingAnchor)
    DECODE() { return decodeEnum(v, out); }

    ENCODE() { return encodeEnum(out, v); }
CODEC_END()

} // namespace mg
