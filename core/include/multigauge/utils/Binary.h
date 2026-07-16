#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

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

inline bool read_u16(const uint8_t* data, size_t size, size_t off, uint16_t& out) {
    if (!data) return false;
    if (off + 2 > size) return false;

    out = (uint16_t)data[off]
        | ((uint16_t)data[off + 1] << 8);
    return true;
}

inline bool read_u32(const uint8_t* data, size_t size, size_t off, uint32_t& out) {
    if (!data) return false;
    if (off + 4 > size) return false;

    out = (uint32_t)data[off]
        | ((uint32_t)data[off + 1] << 8)
        | ((uint32_t)data[off + 2] << 16)
        | ((uint32_t)data[off + 3] << 24);
    return true;
}

inline bool read_i32(const uint8_t* data, size_t size, size_t off, int32_t& out) {
    uint32_t tmp = 0;
    if (!read_u32(data, size, off, tmp)) return false;
    out = (int32_t)tmp;
    return true;
}

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

} // namespace mg::utils
