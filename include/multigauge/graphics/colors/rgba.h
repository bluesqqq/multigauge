#pragma once

#include <stdint.h>
#include <algorithm>
#include <cmath>

#include <multigauge/utils.h>

struct rgba {
    uint8_t r, g, b, a;

    rgba();

    rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255);

    rgba(const char* s);

    void blend(const rgba& other, float t);

    rgba blended(const rgba& other, float t) const;
};

inline bool parse_hex(const char* s, rgba& out) {
    if (!s) return false;

    trimWhitespace(s);

    // Optional prefixes: # or 0x / 0X
    if (s[0] == '#') {
        s++;
    } else if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
        s += 2;
    }

    size_t len = std::strlen(s);
    if (len == 0) return false;

    auto hexval = [](char c) -> int {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return 10 + (c - 'a');
        if (c >= 'A' && c <= 'F') return 10 + (c - 'A');
        return -1;
    };

    auto byte2 = [&](char hi, char lo) -> int {
        int a = hexval(hi), b = hexval(lo);
        if (a < 0 || b < 0) return -1;
        return (a << 4) | b;
    };

    auto expand4 = [&](char c) -> int {
        int v = hexval(c);
        if (v < 0) return -1;
        return (v << 4) | v;
    };

    if (len == 3 || len == 4) {
        int R = expand4(s[0]);
        int G = expand4(s[1]);
        int B = expand4(s[2]);
        if (R < 0 || G < 0 || B < 0) return false;
        out.r = (uint8_t)R; out.g = (uint8_t)G; out.b = (uint8_t)B;
        if (len == 4) {
            int A = expand4(s[3]);
            if (A < 0) return false;
            out.a = (uint8_t)A;
        } else {
            out.a = 255;
        }
        return true;
    }

    if (len == 6 || len == 8) {
        int R = byte2(s[0], s[1]);
        int G = byte2(s[2], s[3]);
        int B = byte2(s[4], s[5]);
        if (R < 0 || G < 0 || B < 0) return false;
        out.r = (uint8_t)R; out.g = (uint8_t)G; out.b = (uint8_t)B;
        if (len == 8) {
            int A = byte2(s[6], s[7]);
            if (A < 0) return false;
            out.a = (uint8_t)A;
        } else {
            out.a = 255;
        }
        return true;
    }

    return false; // unsupported length
}

inline bool parse_rgb(const char* s, rgba& out) {
    if (!s) return false;

    trimWhitespace(s);

    // 2. Check for "rgb(" or "rgba("
    bool hasAlpha = false;
    if (strncmp(s, "rgba(", 5) == 0) {
        hasAlpha = true;
        s += 5;
    } else if (strncmp(s, "rgb(", 4) == 0) {
        hasAlpha = false;
        s += 4;
    } else {
        return false;
    }

    // 3. Parse three/four numbers (r, g, b, [a])
    char* endptr;
    int32_t channels[4] = {0, 0, 0, 255}; // Default alpha 255
    int count = 0;

    for (int i = 0; i < (hasAlpha ? 4 : 3); ++i) {
        // Skip whitespace before number
        while (isspace(*s)) s++;
        
        // Parse integer
        channels[i] = strtol(s, &endptr, 10);
        if (s == endptr) return false; // No digits found
        s = endptr;

        // Skip whitespace after number
        while (isspace(*s)) s++;

        // Expect delimiter (comma or closing parenthesis)
        if (i < (hasAlpha ? 3 : 2)) {
            if (*s == ',') s++;
            else return false; // Missing comma
        }
    }

    // 4. Validate closing parenthesis
    if (*s != ')') return false;

    // 5. Clamp and assign
    auto clamp = [](int32_t v) -> uint8_t {
        if (v < 0) return 0;
        if (v > 255) return 255;
        return (uint8_t)v;
    };

    out.r = clamp(channels[0]);
    out.g = clamp(channels[1]);
    out.b = clamp(channels[2]);
    out.a = hasAlpha ? clamp(channels[3]) : 255;

    return true;
}


inline rgba rgb(uint8_t r, uint8_t g, uint8_t b) { return rgba(r, g, b); }

static std::pair<const char*, rgba> colorTable[] = {
    {"black", rgb(0, 0, 0)},
    {"grey", rgb(128, 128, 128)},
    {"light-grey", rgb(211, 211, 211)},
    {"white", rgb(255, 255, 255)},
    {"red", rgb(255, 0, 0)},
    {"orange", rgb(255, 165, 0)},
    {"yellow", rgb(255, 255, 0)},
    {"lime", rgb(191, 255, 0)},
    {"green", rgb(0, 255, 0)},
    {"teal", rgb(0, 128, 128)},
    {"aqua", rgb(0, 255, 255)},
    {"blue", rgb(0, 0, 255)},
    {"magenta", rgb(255, 0, 255)},
    {"violet", rgb(127, 0, 255)},
    {"purple", rgb(128, 0, 128)}
};

inline bool parse_keyword(const char* s, rgba& out) {
    for (const auto& [keyword, color] : colorTable) {
        if (strcmp(s, keyword) == 0) {
            out = color;
            return true;
        }
    }
    return false;
}