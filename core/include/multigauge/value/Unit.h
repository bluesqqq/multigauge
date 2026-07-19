#pragma once

#include <cstdint>
#include <string_view>

namespace mg {

using UnitIndex = std::int8_t;
using Measurement = float; ///< How "data" is stored.

inline constexpr UnitIndex BASE_UNIT = 0;

/// @brief Describes one display unit and its conversion from a unit type's base unit.
/// @details `factor` and `offset` convert between base units and this unit:
/// `display = (base * factor) + offset`.
/// The base unit for a `UnitType` is always stored at index 0 and should use
/// `factor == 1.0F` and `offset == 0.0F`.
struct Unit {
    std::string_view name = ""; ///< Stable human-readable unit name.
    std::string_view abbreviation = ""; ///< Compact unit label for formatted values.
    Measurement factor = 1.0F; ///< Multiplicative conversion factor from base units to this unit.
    Measurement offset = 0.0F; ///< Additive conversion offset from base units to this unit.
    std::uint8_t decimalPlaces = 0; ///< Default number of decimal places for display formatting.
};

} // namespace mg
