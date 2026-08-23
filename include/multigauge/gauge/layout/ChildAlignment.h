#pragma once

#include <cstdint>

#include <multigauge/properties/Codec.h>
#include <multigauge/properties/EnumTraits.h>
#include <multigauge/properties/PropertyObject.h>

namespace mg::gauge::layout {

/// @brief Defines horizontal alignment for children in a layout container.
enum class AlignmentX : std::uint8_t { Left, Center, Right };

/// @brief Defines vertical alignment for children in a layout container.
enum class AlignmentY : std::uint8_t { Top, Center, Bottom };

/// @brief Stores the cross-axis alignment of a layout container's children.
struct ChildAlignment : ::mg::PropertyObject {
    AlignmentX x = AlignmentX::Left;
    AlignmentY y = AlignmentY::Top;

    MG_PROPS_BEGIN()
        MG_PROP(x, "x", "Horizontal", "Horizontal child alignment.")
        MG_PROP(y, "y", "Vertical", "Vertical child alignment.")
    MG_PROPS_END()
};

} // namespace mg::gauge::layout

namespace mg {

template <> struct EnumTraits<gauge::layout::AlignmentX> {
    static constexpr EnumOption<gauge::layout::AlignmentX> options[] = {
        {gauge::layout::AlignmentX::Left, "left", "Left"},
        {gauge::layout::AlignmentX::Center, "center", "Center"},
        {gauge::layout::AlignmentX::Right, "right", "Right"},
    };
};

template <> struct EnumTraits<gauge::layout::AlignmentY> {
    static constexpr EnumOption<gauge::layout::AlignmentY> options[] = {
        {gauge::layout::AlignmentY::Top, "top", "Top"},
        {gauge::layout::AlignmentY::Center, "center", "Center"},
        {gauge::layout::AlignmentY::Bottom, "bottom", "Bottom"},
    };
};

CODEC_BEGIN(gauge::layout::AlignmentX)
    DECODE() { return decodeEnum(v, out); }

    ENCODE() { return encodeEnum(out, v); }
CODEC_END()

CODEC_BEGIN(gauge::layout::AlignmentY)
    DECODE() { return decodeEnum(v, out); }

    ENCODE() { return encodeEnum(out, v); }
CODEC_END()

} // namespace mg
