#pragma once

#include <cstddef>
#include <string_view>

namespace mg::utils {

/// @brief Returns true if c is a path separator.
static inline bool isPathSeparator(char c) { return c == '/' || c == '\\'; }

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
