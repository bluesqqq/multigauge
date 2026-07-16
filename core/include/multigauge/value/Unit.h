#pragma once

#include <stdint.h>

namespace mg {

struct Unit {
    const char* name;
    const char* abbreviation;
    float factor;
    float offset;
    uint8_t decimalPlaces;
};

inline constexpr int DEFAULT_UNIT = -1;

}
