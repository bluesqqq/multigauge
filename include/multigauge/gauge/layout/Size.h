#pragma once

#include <cstdint>

#include <multigauge/properties/Codec.h>
#include <multigauge/properties/EnumTraits.h>
#include <multigauge/properties/PropertyObject.h>

namespace mg::gauge::layout {

/// @brief Defines the sizing behavior for one layout axis.
enum class SizeMode : std::uint8_t { Fit, Grow, Fixed, Percent };

/// @brief Stores Clay sizing options for one layout axis.
struct Size : ::mg::PropertyObject {
    SizeMode mode = SizeMode::Grow;
    float value = 0.0f;
    float limit = 0.0f;

    MG_PROPS_BEGIN()
        MG_PROP(mode, "mode", "Mode", "Clay sizing mode.")
        MG_PROP(value, "value", "Value", "Fixed size, percent, or minimum size.")
        MG_PROP(limit, "limit", "Limit", "Maximum size for fit or grow sizing.")
    MG_PROPS_END()
};

} // namespace mg::gauge::layout

namespace mg {

template <> struct EnumTraits<gauge::layout::SizeMode> {
    static constexpr EnumOption<gauge::layout::SizeMode> options[] = {
        {gauge::layout::SizeMode::Fit, "fit", "Fit"},
        {gauge::layout::SizeMode::Grow, "grow", "Grow"},
        {gauge::layout::SizeMode::Fixed, "fixed", "Fixed"},
        {gauge::layout::SizeMode::Percent, "percent", "Percent"},
    };
};

CODEC_BEGIN(gauge::layout::SizeMode)
    DECODE() { return decodeEnum(v, out); }

    ENCODE() { return encodeEnum(out, v); }
CODEC_END()

} // namespace mg
