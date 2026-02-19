#pragma once

#include <stdint.h>

struct Unit {
    const char* name;
    const char* abbreviation;
    float factor;
    float offset;
    uint8_t decimalPlaces;
};

#define DEFAULT_UNIT -1