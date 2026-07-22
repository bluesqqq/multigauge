#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <variant>

#include <multigauge/properties/Codec.h>
#include <multigauge/text/TextBuffer.h>
#include <multigauge/value/UnitType.h>
#include <multigauge/value/Value.h>

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

inline bool identifierStart(char c) noexcept {
    return asciiAlpha(c) || c == '_';
}

inline bool identifierCharacter(char c) noexcept {
    return asciiAlnum(c) || c == '_';
}

inline bool validIdentifier(std::string_view input) noexcept {
    if (input.empty() || !identifierStart(input.front())) {
        return false;
    }

    for (size_t i = 1; i < input.size(); ++i) {
        if (!identifierCharacter(input[i])) {
            return false;
        }
    }

    return true;
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
    ::mg::Value* value = nullptr;
    UnitIndex unit = BASE_UNIT;
    int8_t decimals = -1;
    uint8_t flags = None;
};

/*
 * Environment variable names remain inside EmbeddedText::source_.
 *
 * This metadata stores only the name's offset and length, keeping the
 * alternative at four bytes.
 */
struct EnvironmentEmbedMetadata {
    uint16_t nameOffset = 0;
    uint16_t nameLength = 0;
};

using EmbedMetadata = std::variant<
    ValueEmbedMetadata,
    EnvironmentEmbedMetadata
>;

using EnvironmentResolver = bool (*)(
    std::string_view name,
    TextBuffer& out
) noexcept;

inline bool appendValue(
    const ValueEmbedMetadata& metadata,
    TextBuffer& out
) noexcept {
    ::mg::Value* value = metadata.value;

    if (!value) {
        return false;
    }

    const bool fullName =
        (metadata.flags & NoAbbreviation) != 0;

    const bool unitOnly =
        (metadata.flags & UnitOnly) != 0;

    const ::mg::Unit* selected =
        value->unitType().unit(metadata.unit);

    const ::mg::Unit& unit =
        selected
            ? *selected
            : value->unitType().baseUnit();

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

        return out.appendFloat(
                   value->interpolationValue() * 100.0f,
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
            ? value->minimum(metadata.unit)
            : (metadata.flags & Maximum) != 0
                ? value->maximum(metadata.unit)
                : value->value(metadata.unit);

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

    metadata.value = ::mg::Value::find(spec.name);

    if (!metadata.value) {
        return false;
    }

    /*
     * Reject invalid unit indices instead of relying on separate fallback
     * behavior in Value and UnitType.
     */
    if (!metadata.value->unitType().unit(spec.unit)) {
        return false;
    }

    metadata.unit = spec.unit;
    metadata.decimals = spec.decimals;
    metadata.flags = spec.flags;

    return true;
}

inline bool parseEnvironment(
    std::string_view inner,
    size_t absoluteInnerOffset,
    EnvironmentEmbedMetadata& metadata
) noexcept {
    constexpr std::string_view Prefix = "env:";

    if (!inner.starts_with(Prefix)) {
        return false;
    }

    const std::string_view name = inner.substr(Prefix.size());

    if (!validIdentifier(name)) {
        return false;
    }

    const size_t nameOffset =
        absoluteInnerOffset + Prefix.size();

    if (nameOffset > std::numeric_limits<uint16_t>::max() ||
        name.size() > std::numeric_limits<uint16_t>::max()) {
        return false;
    }

    metadata.nameOffset =
        static_cast<uint16_t>(nameOffset);

    metadata.nameLength =
        static_cast<uint16_t>(name.size());

    return true;
}

inline bool parseEmbed(
    std::string_view inner,
    size_t absoluteInnerOffset,
    EmbedMetadata& metadata
) noexcept {
    EnvironmentEmbedMetadata environment;

    if (parseEnvironment(
            inner,
            absoluteInnerOffset,
            environment)) {
        metadata = environment;
        return true;
    }

    ValueEmbedMetadata value;

    if (parseValue(inner, value)) {
        metadata = value;
        return true;
    }

    return false;
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

    /*
     * Renders value embeds only.
     *
     * Returns false if the text contains an environment embed because no
     * environment resolver was supplied.
     */
    [[nodiscard]]
    bool render(TextBuffer& out) const noexcept {
        return render(out, nullptr);
    }

    /*
     * Environment variables use the following syntax:
     *
     *     {env:carName}
     *     {env:gaugeLabel}
     *
     * The resolver writes the corresponding runtime string into `out`.
     */
    [[nodiscard]]
    bool render(
        TextBuffer& out,
        detail::EnvironmentResolver environmentResolver
    ) const noexcept {
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

            const bool success = std::visit(
                [&](const auto& metadata) noexcept -> bool {
                    using T = std::remove_cvref_t<
                        decltype(metadata)
                    >;

                    if constexpr (
                        std::is_same_v<
                            T,
                            detail::ValueEmbedMetadata
                        >
                    ) {
                        return detail::appendValue(
                            metadata,
                            out
                        );
                    } else {
                        if (!environmentResolver) {
                            return false;
                        }

                        const std::string_view name =
                            source.substr(
                                metadata.nameOffset,
                                metadata.nameLength
                            );

                        return environmentResolver(name, out);
                    }
                },
                embeds_[i]
            );

            if (!success) {
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

            detail::EmbedMetadata metadata;

            const size_t innerOffset = cursor + 1;

            const std::string_view inner =
                source.substr(
                    innerOffset,
                    close - innerOffset
                );

            if (!detail::parseEmbed(
                    inner,
                    innerOffset,
                    metadata)) {
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
        detail::EmbedMetadata,
        MaxEmbeds
    > embeds_{};

    uint8_t embedCount_ = 0;
    bool literalFallback_ = false;
};

} // namespace mg::text

namespace mg {

CODEC_BEGIN(text::EmbeddedText)
    DECODE() {
        if (!v.IsString()) {
            return false;
        }

        out.setSource(std::string(
            v.GetString(),
            v.GetStringLength()
        ));

        return true;
    }

    ENCODE() {
        out.SetString(
            v.source().data(),
            static_cast<rapidjson::SizeType>(
                v.source().size()
            ),
            a
        );

        return true;
    }
CODEC_END()

} // namespace mg