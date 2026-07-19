#pragma once

#include <cstdint>
#include <string_view>

namespace mg {

using UnitIndex = std::int8_t;
using Measurement = float; ///< How "data" is stored.

inline constexpr UnitIndex BASE_UNIT = 0;

struct Unit {
    std::string_view name = "";
    std::string_view abbreviation = "";
    Measurement factor = 1.0F;
    Measurement offset = 0.0F;
    std::uint8_t decimalPlaces = 0;
};

} // namespace mg
