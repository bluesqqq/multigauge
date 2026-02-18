#pragma once

#include <cstdint>     // uint8_t, uint16_t, uint32_t, int32_t
#include <cstddef>     // size_t
#include <string>      // std::string
#include <algorithm>   // std::min
#include <rapidjson/document.h>
#include <optional>
#include <vector>

#include <multigauge/io/Logger.h>

// ----------[ FLOAT / MATH ]---------- //

size_t fastFloatToString(float value, uint8_t decimalPlaces, char* buf, size_t bufSize);

std::string floatToString(float value, uint8_t decimalPlaces);

float mapf(float x, float inMin, float inMax, float outMin, float outMax);

template <typename T>
static inline T lerp(T a, T b, float t) {
    return a + (b - a) * t;
}

float floorDivisible(float n, float factor, float offset = 0);

float ceilDivisible(float n, float factor, float offset = 0);

inline bool inRange(float v, float lo, float hi) { return (v >= lo) && (v <= hi); }

// ----------[ BINARY READERS ]---------- //

bool read_u16(const uint8_t* data, size_t size, size_t off, uint16_t& out);

bool read_u32(const uint8_t* data, size_t size, size_t off, uint32_t& out);

bool read_i32(const uint8_t* data, size_t size, size_t off, int32_t& out);

// ----------[ BIT HELPERS ]---------- //
// Header-only helpers MUST be inline (or static) to avoid multiple definition errors.

static inline int count_trailing_zeros_u32(uint32_t v) {
    if (v == 0) return 32;
    int c = 0;
    while ((v & 1u) == 0u) { v >>= 1u; ++c; }
    return c;
}

static inline int count_bits_u32(uint32_t v) {
    int c = 0;
    while (v) { v &= (v - 1u); ++c; }
    return c;
}

static inline uint8_t extract_and_scale(uint32_t value, uint32_t mask) {
    if (mask == 0) return 0;

    const int shift = count_trailing_zeros_u32(mask);
    const int bits  = count_bits_u32(mask);

    const uint32_t raw = (value & mask) >> shift;
    if (bits <= 0) return 0;

    const uint32_t maxv = (bits >= 32) ? 0xFFFFFFFFu : ((1u << bits) - 1u);
    if (maxv == 0) return 0;

    const uint32_t scaled = (raw * 255u + (maxv / 2u)) / maxv;
    return (uint8_t)std::min<uint32_t>(scaled, 255u);
}

// ----------[ COLOR ]---------- //

uint16_t rgb888_to_565(uint8_t r, uint8_t g, uint8_t b);

// ----------[ PARSERS ]---------- //

/// @brief Parses an unsigned decimal integer from s starting at i.
/// @param s The input view.
/// @param i In/out cursor. Advances past parsed digits.
/// @param out Output integer (clamped to 32767).
/// @return true if at least one digit was parsed.
static inline bool parseUnsignedInt(std::string_view s, size_t& i, int& out) {
    if (i >= s.size() || s[i] < '0' || s[i] > '9') return false;
    int v = 0;
    while (i < s.size()) {
        char c = s[i];
        if (c < '0' || c > '9') break;
        v = (v * 10) + (c - '0');
        i++;
        if (v > 32767) v = 32767;
    }
    out = v;
    return true;
}

// ----------[ SPLITTERS ]---------- //

/// @brief Splits s on the first occurrence of delim without allocating.
/// @param s Input.
/// @param delim Delimiter.
/// @param left Output left side.
/// @param right Output right side (empty if no delim).
/// @param hasDelim True if delim was found.
static inline void splitOnce(std::string_view s, char delim, std::string_view& left, std::string_view& right, bool& hasDelim) {
    size_t p = s.find(delim);
    if (p == std::string_view::npos) {
        left = s;
        right = {};
        hasDelim = false;
        return;
    }
    left = s.substr(0, p);
    right = s.substr(p + 1);
    hasDelim = true;
}

inline void trimLeadingWhitespace(const char* s) {
    while (isspace(*s)) ++s;
}

inline void trimTrailingWhitespace(const char* s) {
    const char* end = s + std::strlen(s);
    while (end > s && (isspace(end[-1]))) --end;
}

inline void trimWhitespace(const char* s) {
    trimLeadingWhitespace(s);
    trimTrailingWhitespace(s);
}