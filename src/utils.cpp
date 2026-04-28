#include <multigauge/utils.h>

#include <cstdio>
#include <cmath>

namespace mg {

// ----------[ FLOAT / STRING ]---------- //

size_t fastFloatToString(float value, uint8_t decimalPlaces, char* buf, size_t bufSize) {
    if (bufSize == 0) return 0;

    char* p = buf;

    // Handle negative
    if (value < 0.0f) {
        if ((size_t)(p - buf) < bufSize - 1) *p++ = '-';
        value = -value;
    }

    // Compute rounding factor
    float rounding = 0.5f;
    for (uint8_t i = 0; i < decimalPlaces; i++) rounding /= 10.0f;

    value += rounding;

    // Extract integer part
    int32_t intPart = (int32_t)value;
    float fracPart = value - (float)intPart;

    // Write integer digits
    char intBuf[12];
    char* q = intBuf + sizeof(intBuf) - 1;
    *q = '\0';

    if (intPart == 0) {
        *--q = '0';
    } else {
        while (intPart > 0) {
            *--q = char('0' + (intPart % 10));
            intPart /= 10;
        }
    }

    while (*q && (size_t)(p - buf) < bufSize - 1) {
        *p++ = *q++;
    }

    // Decimal point
    if (decimalPlaces > 0 && (size_t)(p - buf) < bufSize - 1) {
        *p++ = '.';
    }

    // Write fractional digits
    for (uint8_t i = 0; i < decimalPlaces; i++) {
        fracPart *= 10.0f;
        int digit = (int)fracPart;
        if ((size_t)(p - buf) < bufSize - 1) {
            *p++ = char('0' + digit);
        }
        fracPart -= digit;
    }

    *p = '\0';
    return (size_t)(p - buf);
}

std::string floatToString(float value, uint8_t decimalPlaces) {
    char buf[16];
    fastFloatToString(value, decimalPlaces, buf, sizeof(buf));
    return std::string(buf);
}

float mapf(float x, float inMin, float inMax, float outMin, float outMax) {
    if (inMin == inMax) return outMin;
    return (x - inMin) * (outMax - outMin) / (inMax - inMin) + outMin;
}

// ----------[ BINARY READERS ]---------- //

float floorDivisible(float n, float factor, float offset) { return std::floor((n - offset) / factor) * factor + offset; }

float ceilDivisible(float n, float factor, float offset) { return std::ceil((n - offset) / factor * factor + offset); }

bool read_u16(const uint8_t *data, size_t size, size_t off, uint16_t &out)
{
    if (!data) return false;
    if (off + 2 > size) return false;

    out = (uint16_t)data[off]
        | ((uint16_t)data[off + 1] << 8);
    return true;
}

bool read_u32(const uint8_t* data, size_t size, size_t off, uint32_t& out) {
    if (!data) return false;
    if (off + 4 > size) return false;

    out = (uint32_t)data[off]
        | ((uint32_t)data[off + 1] << 8)
        | ((uint32_t)data[off + 2] << 16)
        | ((uint32_t)data[off + 3] << 24);
    return true;
}

bool read_i32(const uint8_t* data, size_t size, size_t off, int32_t& out) {
    uint32_t tmp = 0;
    if (!read_u32(data, size, off, tmp)) return false;
    out = (int32_t)tmp;
    return true;
}

// ----------[ COLOR ]---------- //

uint16_t rgb888_to_565(uint8_t r, uint8_t g, uint8_t b) {
    // 5-6-5
    return (uint16_t)(((r & 0xF8) << 8) |
                      ((g & 0xFC) << 3) |
                      ((b & 0xF8) >> 3));
}

} // namespace mg
