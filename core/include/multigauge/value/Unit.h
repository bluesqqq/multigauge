#pragma once

#include <cstdint>
#include <string_view>

namespace mg {

using UnitIndex = std::int8_t;

inline constexpr UnitIndex DEFAULT_UNIT = -1;

struct Unit {
    std::string_view name = "";
    std::string_view abbreviation = "";
    float factor = 1.0F;
    float offset = 0.0F;
    std::uint8_t decimalPlaces = 0;
};

} // namespace mg
