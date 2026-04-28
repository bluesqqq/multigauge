#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace mg::utils {

namespace detail {

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

} // namespace detail

size_t fastFloatToString(float value, uint8_t decimalPlaces, char* buf, size_t bufSize);

std::string floatToString(float value, uint8_t decimalPlaces);

float mapf(float x, float inMin, float inMax, float outMin, float outMax);

template <typename T>
inline T lerp(T a, T b, float t) {
    return a + (b - a) * t;
}

float floorDivisible(float n, float factor, float offset = 0);

float ceilDivisible(float n, float factor, float offset = 0);

inline bool inRange(float v, float lo, float hi) { return (v >= lo) && (v <= hi); }

bool read_u16(const uint8_t* data, size_t size, size_t off, uint16_t& out);

bool read_u32(const uint8_t* data, size_t size, size_t off, uint32_t& out);

bool read_i32(const uint8_t* data, size_t size, size_t off, int32_t& out);

inline uint8_t extract_and_scale(uint32_t value, uint32_t mask) {
    if (mask == 0) return 0;

    const int shift = detail::count_trailing_zeros_u32(mask);
    const int bits = detail::count_bits_u32(mask);
    if (bits <= 0) return 0;

    const uint32_t raw = (value & mask) >> shift;
    const uint32_t maxv = (bits >= 32) ? 0xFFFFFFFFu : ((1u << bits) - 1u);
    if (maxv == 0) return 0;

    const uint32_t scaled = (raw * 255u + (maxv / 2u)) / maxv;
    return (uint8_t)std::min<uint32_t>(scaled, 255u);
}

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

} // namespace mg::utils
