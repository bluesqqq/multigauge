#pragma once

#include <string>
#include <string_view>
#include <cstdint>
#include <cstdio>

#include "Value.h"
#include "units.h"
#include "utils.h"

namespace embed {

// --------------------[ Flags + Spec ]--------------------

/// @brief Bit flags that control how an embed is rendered.
enum Flag : uint8_t {
    FLAG_NONE = 0,
    FLAG_NA   = 1 << 0,  ///< "na"  -> no abbreviation (use full unit name)
    FLAG_U    = 1 << 1,  ///< "u"   -> unit only (no number)
    FLAG_MIN  = 1 << 2,  ///< "min" -> show minimum
    FLAG_MAX  = 1 << 3,  ///< "max" -> show maximum
    FLAG_PCT  = 1 << 4   ///< "pct" -> show percentage (0..100)
};

/// @brief Parse errors for the inner embed grammar.
enum class ParseError : uint8_t {
    None = 0,
    Empty,         ///< empty inner text
    MissingName,   ///< missing value name
    BadNameChar,   ///< name doesn't start with [A-Za-z_]
    BadUnitIndex,  ///< "=..." was present but not digits
    BadDecimals,   ///< "...." was present but not digits
    BadFlags,      ///< flags contained whitespace or bad separators
    TrailingJunk   ///< leftover chars after parsing lhs
};

/// @brief Parsed representation of "{...}" inner text. Holds string_views into the source.
struct EmbedSpec {
    std::string_view valueName;   ///< required identifier
    int16_t unitIndex  = -1;      ///< optional, after '=' (>=0). -1 means unspecified.
    int8_t  decimals   = -1;      ///< optional, after '.' (>=0). -1 means unspecified.
    uint8_t flags      = FLAG_NONE;

    ParseError error   = ParseError::None;
    bool ok() const { return error == ParseError::None; }
};

// --------------------[ Character helpers ]--------------------

/// @brief Compares token [start,end) in s with a null-terminated ASCII literal.
/// @return true if exactly equal.
static inline bool tokEq(std::string_view s, size_t start, size_t end, const char* lit) {
    size_t n = end - start;
    for (size_t k = 0; k < n; k++) {
        if (lit[k] == '\0') return false;
        if (s[start + k] != lit[k]) return false;
    }
    return lit[n] == '\0';
}

// --------------------[ Flag parsing (tight) ]--------------------

static inline void addFlagToken(uint8_t& flags, std::string_view rhs, size_t start, size_t end) {
    if (end <= start) return;

    if (tokEq(rhs, start, end, "na"))       flags |= FLAG_NA;
    else if (tokEq(rhs, start, end, "u"))  flags |= FLAG_U;
    else if (tokEq(rhs, start, end, "min")) flags |= FLAG_MIN;
    else if (tokEq(rhs, start, end, "max")) flags |= FLAG_MAX;
    else if (tokEq(rhs, start, end, "pct")) flags |= FLAG_PCT;
    // Unknown tokens are ignored by design.
}

static inline void parseFlagsTight(uint8_t& flags, std::string_view rhs, ParseError& err) {
    if (rhs.empty()) return;

    // Reject any whitespace anywhere in flags.
    for (char c : rhs) {
        if (isspace(c)) { err = ParseError::BadFlags; return; }
    }

    // Split on commas. Empty tokens are rejected.
    size_t i = 0;
    while (i < rhs.size()) {
        size_t start = i;

        while (i < rhs.size() && rhs[i] != ',') i++;
        size_t end = i;

        if (end == start) { err = ParseError::BadFlags; return; } // ",," or leading/trailing comma

        addFlagToken(flags, rhs, start, end);

        if (i < rhs.size() && rhs[i] == ',') i++; // skip comma
    }
}

// --------------------[ Main parse (tight) ]--------------------

static inline EmbedSpec parseEmbedInnerTight(std::string_view s) {
    EmbedSpec spec;

    if (s.empty()) { spec.error = ParseError::Empty; return spec; }

    for (char c : s) {
        if (isspace(c)) { spec.error = ParseError::TrailingJunk; return spec; }
    }

    std::string_view lhs, rhs;
    bool hasBar = false;
    splitOnce(s, '|', lhs, rhs, hasBar);

    size_t i = 0;

    if (lhs.empty()) { spec.error = ParseError::MissingName; return spec; }
    if (!isalpha(lhs[0])) { spec.error = ParseError::BadNameChar; return spec; }

    while (i < lhs.size() && isalnum(lhs[i])) i++;
    spec.valueName = lhs.substr(0, i);
    if (spec.valueName.empty()) { spec.error = ParseError::MissingName; return spec; }

    if (i < lhs.size() && lhs[i] == '=') {
        i++;
        int v = 0;
        if (!parseUnsignedInt(lhs, i, v)) { spec.error = ParseError::BadUnitIndex; return spec; }
        spec.unitIndex = (v > 32767) ? 32767 : (int16_t)v;
    }

    if (i < lhs.size() && lhs[i] == '.') {
        i++;
        int d = 0;
        if (!parseUnsignedInt(lhs, i, d)) { spec.error = ParseError::BadDecimals; return spec; }
        spec.decimals = (d > 127) ? 127 : (int8_t)d;
    }

    if (i != lhs.size()) { spec.error = ParseError::TrailingJunk; return spec; }

    if (hasBar) {
        ParseError ferr = ParseError::None;
        parseFlagsTight(spec.flags, rhs, ferr);
        if (ferr != ParseError::None) { spec.error = ferr; return spec; }
    }

    spec.error = ParseError::None;
    return spec;
}

} // namespace embed

namespace embed_render {

// --------------------[ render helpers ]--------------------

static inline void appendRange(std::string& out, const char* begin, const char* end) {
    out.append(begin, static_cast<size_t>(end - begin));
}

static inline void appendFloat(std::string& out, float v, uint8_t decimals) {
    char buf[32];
    size_t len = fastFloatToString(v, decimals, buf, sizeof(buf));
    if (len) out.append(buf, len);
}

static inline std::string getUnitString(const UnitType& ut, int unitIndex, bool abbreviation) {
    const Unit& u = ut.getUnit(unitIndex);
    const char* s = abbreviation ? u.abbreviation : u.name;
    return (s && *s) ? std::string(s) : std::string();
}

// --------------------[ render one embed ]--------------------

/// @brief Renders a single embed inner string into out.
/// @param inner Inner embed string (no braces).
/// @param out Output string (cleared and written on success).
/// @return true if parse + lookup + render succeeded, false otherwise.
static inline bool renderOne(std::string_view inner, std::string& out) {
    out.clear();

    embed::EmbedSpec spec = embed::parseEmbedInnerTight(inner);
    if (!spec.ok()) return false;

    // NOTE: replace with Value::find(spec.valueName) once you have a string_view overload.
    Value* v = Value::find(std::string(spec.valueName));
    if (!v) return false;

    const bool flagNA  = (spec.flags & embed::FLAG_NA)  != 0;
    const bool flagU   = (spec.flags & embed::FLAG_U)   != 0;
    const bool flagMIN = (spec.flags & embed::FLAG_MIN) != 0;
    const bool flagMAX = (spec.flags & embed::FLAG_MAX) != 0;
    const bool flagPCT = (spec.flags & embed::FLAG_PCT) != 0;

    const bool useAbbrev = !flagNA;

    int unitIndex = (spec.unitIndex >= 0) ? spec.unitIndex : DEFAULT_UNIT;

    if (flagPCT) { // PERCENTAGE
        if (flagU) {
            out = useAbbrev ? "%" : "percent";
            return true;
        }

        float pct = v->getInterpolationValue() * 100.0f;
        uint8_t decimals = (spec.decimals >= 0) ? (uint8_t)spec.decimals : 0;

        appendFloat(out, pct, decimals);
        out += (useAbbrev ? "%" : " percent");
        return true;
    }

    if (flagU) { // UNIT ONLY
        out = getUnitString(v->getUnitType(), unitIndex, useAbbrev);
        return !out.empty();
    }

    // choose which numeric value to show
    double shown = 0.0;
    if (flagMIN)      shown = v->getMinimum(unitIndex);
    else if (flagMAX) shown = v->getMaximum(unitIndex);
    else              shown = v->getValue(unitIndex);

    // choose decimals
    int decimals = 0;
    if (spec.decimals >= 0) decimals = spec.decimals;
    else decimals = (int)v->getUnitType().getUnit(unitIndex).decimalPlaces;

    appendFloat(out, shown, decimals);

    // append unit if present
    if (useAbbrev) {
        const char* ab = v->getUnitType().getUnit(unitIndex).abbreviation;
        if (ab && *ab) out += ab; // no space
    } else {
        const char* nm = v->getUnitType().getUnit(unitIndex).name;
        if (nm && *nm) { out += " "; out += nm; }
    }

    return true;
}

// --------------------[ replace across full string ]--------------------

static inline std::string replaceEmbeds(const std::string& input) {
    if (input.find('{') == std::string::npos) return input;

    std::string out;
    out.reserve(input.size());

    const char* s = input.data();
    const size_t n = input.size();

    size_t i = 0;
    size_t lastCopy = 0;

    std::string rendered;
    rendered.reserve(32);

    while (i < n) {
        if (s[i] != '{') { i++; continue; }

        appendRange(out, s + lastCopy, s + i);

        size_t open = i;
        size_t j = i + 1;
        while (j < n && s[j] != '}') j++;

        if (j >= n) {
            appendRange(out, s + open, s + n);
            return out;
        }

        std::string_view inner(s + open + 1, j - (open + 1));

        if (renderOne(inner, rendered)) out += rendered;
        else appendRange(out, s + open, s + (j + 1));

        i = j + 1;
        lastCopy = i;
    }

    appendRange(out, s + lastCopy, s + n);
    return out;
}

} // namespace embed_render
