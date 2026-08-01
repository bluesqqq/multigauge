#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <utility>

#include <multigauge/properties/Codec.h>
#include <multigauge/text/TextBuffer.h>
#include <multigauge/value/UnitType.h>
#include <multigauge/value/ValueRegistry.h>

namespace mg::text {

namespace detail {

enum Flag : uint8_t {
    None           = 0,
    NoAbbreviation = 1 << 0,
    UnitOnly       = 1 << 1,
    Minimum        = 1 << 2,
    Maximum        = 1 << 3,
    Percentage     = 1 << 4
};

enum class ParseError : uint8_t {
    None,
    Empty,
    MissingName,
    BadName,
    BadUnit,
    BadDecimals,
    BadFlags,
    TrailingJunk
};

struct Spec {
    std::string_view name;
    UnitIndex unit = BASE_UNIT;
    int8_t decimals = -1;
    uint8_t flags = None;
    ParseError error = ParseError::None;
};

inline bool asciiSpace(char c) noexcept {
    return c == ' ' || c == '\t' || c == '\n' ||
           c == '\r' || c == '\f' || c == '\v';
}

inline bool asciiAlpha(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ||
           (c >= 'a' && c <= 'z');
}

inline bool asciiDigit(char c) noexcept {
    return c >= '0' && c <= '9';
}

inline bool asciiAlnum(char c) noexcept {
    return asciiAlpha(c) || asciiDigit(c);
}

inline Spec parseValueSpec(std::string_view input) noexcept {
    Spec spec;

    if (input.empty()) {
        spec.error = ParseError::Empty;
        return spec;
    }

    for (char c : input) {
        if (asciiSpace(c)) {
            spec.error = ParseError::TrailingJunk;
            return spec;
        }
    }

    const size_t bar = input.find('|');

    const std::string_view lhs = input.substr(0, bar);
    const std::string_view flags =
        bar == std::string_view::npos
            ? std::string_view{}
            : input.substr(bar + 1);

    if (lhs.empty()) {
        spec.error = ParseError::MissingName;
        return spec;
    }

    if (!asciiAlpha(lhs.front())) {
        spec.error = ParseError::BadName;
        return spec;
    }

    size_t i = 0;

    while (i < lhs.size() && asciiAlnum(lhs[i])) {
        ++i;
    }

    spec.name = lhs.substr(0, i);

    if (i < lhs.size() && lhs[i] == '=') {
        ++i;

        int value = 0;
        const size_t start = i;

        while (i < lhs.size() && asciiDigit(lhs[i])) {
            value = value > 3276
                ? 32767
                : value * 10 + lhs[i] - '0';

            ++i;
        }

        if (i == start ||
            value > std::numeric_limits<UnitIndex>::max()) {
            spec.error = ParseError::BadUnit;
            return spec;
        }

        spec.unit = static_cast<UnitIndex>(value);
    }

    if (i < lhs.size() && lhs[i] == '.') {
        ++i;

        int decimals = 0;
        const size_t start = i;

        while (i < lhs.size() && asciiDigit(lhs[i])) {
            decimals = decimals > 3276
                ? 32767
                : decimals * 10 + lhs[i] - '0';

            ++i;
        }

        if (i == start || decimals > 9) {
            spec.error = ParseError::BadDecimals;
            return spec;
        }

        spec.decimals = static_cast<int8_t>(decimals);
    }

    if (i != lhs.size()) {
        spec.error = ParseError::TrailingJunk;
        return spec;
    }

    if (bar == std::string_view::npos) {
        return spec;
    }

    size_t start = 0;

    while (start <= flags.size()) {
        const size_t comma = flags.find(',', start);

        const size_t end =
            comma == std::string_view::npos
                ? flags.size()
                : comma;

        const std::string_view token =
            flags.substr(start, end - start);

        if (token.empty()) {
            spec.error = ParseError::BadFlags;
            return spec;
        }

        if (token == "na") {
            spec.flags |= NoAbbreviation;
        } else if (token == "u") {
            spec.flags |= UnitOnly;
        } else if (token == "min") {
            spec.flags |= Minimum;
        } else if (token == "max") {
            spec.flags |= Maximum;
        } else if (token == "pct") {
            spec.flags |= Percentage;
        } else {
            spec.error = ParseError::BadFlags;
            return spec;
        }

        if (comma == std::string_view::npos) {
            break;
        }

        start = comma + 1;
    }

    if ((spec.flags & Minimum) != 0 &&
        (spec.flags & Maximum) != 0) {
        spec.error = ParseError::BadFlags;
    }

    return spec;
}

/*
 * Offset and length are limited to 16 bits.
 *
 * EmbeddedText falls back to rendering the source directly when the source
 * exceeds this representable range.
 */
struct StringPart {
    uint16_t offset = 0;
    uint16_t length = 0;
};

struct ValueEmbedMetadata {
    ::mg::ValueHandle value;
    UnitIndex unit = BASE_UNIT;
    int8_t decimals = -1;
    uint8_t flags = None;
};

inline bool appendValue(
    const ValueEmbedMetadata& metadata,
    TextBuffer& out
) noexcept {
    if (!::mg::ValueRegistry::exists(metadata.value)) {
        return false;
    }

    const ::mg::UnitType* unitType = ::mg::ValueRegistry::unit(metadata.value);
    if (!unitType) return false;

    const bool fullName =
        (metadata.flags & NoAbbreviation) != 0;

    const bool unitOnly =
        (metadata.flags & UnitOnly) != 0;

    const ::mg::Unit* selected =
        unitType->unit(metadata.unit);

    const ::mg::Unit& unit =
        selected
            ? *selected
            : unitType->baseUnit();

    const std::string_view unitText =
        fullName
            ? unit.name
            : unit.abbreviation;

    if ((metadata.flags & Percentage) != 0) {
        if (unitOnly) {
            return out.append(
                fullName
                    ? std::string_view{"percent"}
                    : std::string_view{"%"}
            );
        }

        const uint8_t decimals =
            metadata.decimals < 0
                ? 0
                : static_cast<uint8_t>(metadata.decimals);

        const float minimum = ::mg::ValueRegistry::minimum(metadata.value);
        const float maximum = ::mg::ValueRegistry::maximum(metadata.value);
        const float span = maximum - minimum;

        return out.appendFloat(
                   (span == 0.0F ? 50.0F :
                    ((::mg::ValueRegistry::value(metadata.value) - minimum) / span) * 100.0F),
                   decimals
               ) &&
               out.append(
                   fullName
                       ? std::string_view{" percent"}
                       : std::string_view{"%"}
               );
    }

    if (unitOnly) {
        return out.append(unitText);
    }

    const float shown =
        (metadata.flags & Minimum) != 0
            ? unitType->convertFromBase(::mg::ValueRegistry::minimum(metadata.value), metadata.unit)
            : (metadata.flags & Maximum) != 0
                ? unitType->convertFromBase(::mg::ValueRegistry::maximum(metadata.value), metadata.unit)
                : unitType->convertFromBase(::mg::ValueRegistry::value(metadata.value), metadata.unit);

    const uint8_t decimals =
        metadata.decimals < 0
            ? unit.decimalPlaces
            : static_cast<uint8_t>(metadata.decimals);

    if (!out.appendFloat(shown, decimals)) {
        return false;
    }

    if (unitText.empty()) {
        return true;
    }

    if (fullName && !out.append(' ')) {
        return false;
    }

    return out.append(unitText);
}

inline bool parseValue(
    std::string_view inner,
    ValueEmbedMetadata& metadata
) noexcept {
    const Spec spec = parseValueSpec(inner);

    if (spec.error != ParseError::None) {
        return false;
    }

    metadata.value = ::mg::ValueRegistry::resolve(spec.name);

    if (!metadata.value.valid()) {
        return false;
    }

    /*
     * Reject invalid unit indices instead of relying on separate fallback
     * behavior in Value and UnitType.
     */
    const ::mg::UnitType* unitType = ::mg::ValueRegistry::unit(metadata.value);
    if (!unitType || !unitType->unit(spec.unit)) {
        return false;
    }

    metadata.unit = spec.unit;
    metadata.decimals = spec.decimals;
    metadata.flags = spec.flags;

    return true;
}

} // namespace detail

class EmbeddedText {
public:
    static constexpr size_t MaxEmbeds = 8;

    EmbeddedText() = default;

    explicit EmbeddedText(std::string source)
        : source_(std::move(source)) {
        rebuildCache();
    }

    EmbeddedText(const EmbeddedText&) = default;
    EmbeddedText(EmbeddedText&&) noexcept = default;

    EmbeddedText& operator=(const EmbeddedText&) = default;
    EmbeddedText& operator=(EmbeddedText&&) noexcept = default;

    [[nodiscard]]
    const std::string& source() const noexcept {
        return source_;
    }

    [[nodiscard]]
    std::string_view sourceView() const noexcept {
        return source_;
    }

    [[nodiscard]]
    bool hasEmbeds() const noexcept {
        return embedCount_ != 0;
    }

    void setSource(std::string source) {
        source_ = std::move(source);
        rebuildCache();
    }

    [[nodiscard]]
    bool render(TextBuffer& out) const noexcept {
        out.clear();

        /*
         * Oversized sources or cache overflow are retained and rendered as
         * plain text without attempting to represent their offsets in
         * uint16_t.
         */
        if (literalFallback_) {
            return out.append(source_) && out.ok();
        }

        const std::string_view source = source_;

        for (size_t i = 0; i < embedCount_; ++i) {
            const detail::StringPart& stringPart =
                strings_[i];

            if (!out.append(source.substr(
                    stringPart.offset,
                    stringPart.length))) {
                return false;
            }

            if (!detail::appendValue(embeds_[i], out)) {
                return false;
            }
        }

        const detail::StringPart& trailing =
            strings_[embedCount_];

        if (!out.append(source.substr(
                trailing.offset,
                trailing.length))) {
            return false;
        }

        return out.ok();
    }

private:
    void rebuildCache() noexcept {
        embedCount_ = 0;
        literalFallback_ = false;

        const std::string_view source = source_;

        if (source.size() >
            std::numeric_limits<uint16_t>::max()) {
            fallBackToLiteral();
            return;
        }

        size_t cursor = 0;
        size_t literalStart = 0;

        while (cursor < source.size()) {
            if (source[cursor] != '{') {
                ++cursor;
                continue;
            }

            const size_t close =
                source.find('}', cursor + 1);

            if (close == std::string_view::npos) {
                break;
            }

            detail::ValueEmbedMetadata metadata;

            const std::string_view inner =
                source.substr(
                    cursor + 1,
                    close - cursor - 1
                );

            if (!detail::parseValue(inner, metadata)) {
                ++cursor;
                continue;
            }

            if (embedCount_ >= MaxEmbeds) {
                fallBackToLiteral();
                return;
            }

            /*
             * Every embed has exactly one literal region before it.
             * Adjacent embeds produce a zero-length literal region.
             */
            strings_[embedCount_] = detail::StringPart{
                static_cast<uint16_t>(literalStart),
                static_cast<uint16_t>(
                    cursor - literalStart
                )
            };

            embeds_[embedCount_] = metadata;
            ++embedCount_;

            cursor = close + 1;
            literalStart = cursor;
        }

        /*
         * N embeds always have N + 1 literal regions. This final entry is
         * the text after the last embed. With no embeds, it contains the
         * entire source.
         */
        strings_[embedCount_] = detail::StringPart{
            static_cast<uint16_t>(literalStart),
            static_cast<uint16_t>(
                source.size() - literalStart
            )
        };
    }

    void fallBackToLiteral() noexcept {
        embedCount_ = 0;
        literalFallback_ = true;
    }

    std::string source_;

    std::array<
        detail::StringPart,
        MaxEmbeds + 1
    > strings_{};

    std::array<
        detail::ValueEmbedMetadata,
        MaxEmbeds
    > embeds_{};

    uint8_t embedCount_ = 0;
    bool literalFallback_ = false;
};

} // namespace mg::text

namespace mg {

CODEC_BEGIN(text::EmbeddedText)
    DECODE() {
        std::string_view source;
        if (!v.read(source)) return false;
        out.setSource(std::string(source));

        return true;
    }

    ENCODE() {
        return out.write(v.source());
    }
CODEC_END()

} // namespace mg
