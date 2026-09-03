#pragma once

#include <cstdint>

#include <multigauge/properties/Codec.h>
#include <multigauge/properties/EnumTraits.h>

namespace mg::gauge::layout {

/// @brief Defines the direction in which child elements are laid out.
enum class Direction : std::uint8_t { Horizontal, Vertical };

} // namespace mg::gauge::layout

namespace mg {

template <> struct EnumTraits<gauge::layout::Direction> {
    static constexpr EnumOption<gauge::layout::Direction> options[] = {
        {gauge::layout::Direction::Horizontal, "horizontal", "Horizontal"},
        {gauge::layout::Direction::Vertical, "vertical", "Vertical"},
    };
};

CODEC_BEGIN(gauge::layout::Direction)
    DECODE() { return decodeEnum(v, out); }

    ENCODE() { return encodeEnum(out, v); }
CODEC_END()

} // namespace mg
