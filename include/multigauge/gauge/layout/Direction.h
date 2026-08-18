#pragma once

#include <cstdint>

#include <multigauge/properties/Codec.h>
#include <multigauge/properties/EnumTraits.h>

namespace mg::gauge::layout {

/// @brief Defines the direction in which child elements are laid out.
enum class Direction : std::uint8_t { LeftToRight, TopToBottom };

} // namespace mg::gauge::layout

namespace mg {

template <> struct EnumTraits<gauge::layout::Direction> {
    static constexpr EnumOption<gauge::layout::Direction> options[] = {
        {gauge::layout::Direction::LeftToRight, "left-to-right", "Left to Right"},
        {gauge::layout::Direction::TopToBottom, "top-to-bottom", "Top to Bottom"},
    };
};

CODEC_BEGIN(gauge::layout::Direction)
    DECODE() { return decodeEnum(v, out); }

    ENCODE() { return encodeEnum(out, v); }
CODEC_END()

} // namespace mg
