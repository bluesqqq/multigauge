#pragma once

#include <multigauge/json/Json.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace mg {

class PropertyObject;
template<typename T> struct Codec;

template <typename T, typename = void> struct HasCodec : std::false_type {};
template <typename T> struct HasCodec<T, std::void_t<
    decltype(Codec<T>::decode(std::declval<json::Reader>(), std::declval<T&>())),
    decltype(Codec<T>::encode(std::declval<json::Writer&>(), std::declval<const T&>()))
>> : std::true_type {};
template <typename T> inline constexpr bool HasCodecV = HasCodec<T>::value;

template <typename T> bool decodeAny(json::Reader value, T& out);
template <typename T> bool encodeAny(json::Writer& writer, const T& value);

template <class T> struct mg_remove_cvref { using type = std::remove_cv_t<std::remove_reference_t<T>>; };
#define CODEC_FRIEND(T) friend struct Codec<T>;
#define CODEC_BEGIN(CODEC_T) template<> struct Codec<CODEC_T> { using CodecType = CODEC_T;
#define CODEC_BEGIN_TPARAMS(TPARAMS, CODEC_T) template<TPARAMS> struct Codec<CODEC_T> { using CodecType = CODEC_T;
#define DECODE() static bool decode(::mg::json::Reader v, CodecType& out)
#define ENCODE() static bool encode(::mg::json::Writer& out, const CodecType& v)
#define CODEC_END() };
#define DECODE_IMPL(CODEC_T) bool Codec<CODEC_T>::decode(::mg::json::Reader v, CodecType& out)
#define ENCODE_IMPL(CODEC_T) bool Codec<CODEC_T>::encode(::mg::json::Writer& out, const CodecType& v)

CODEC_BEGIN(bool)
    DECODE() { return v.read(out); }
    ENCODE() { return out.write(v); }
CODEC_END()

CODEC_BEGIN(int)
    DECODE() { std::int64_t decoded; if (!v.read(decoded) || decoded < std::numeric_limits<int>::min() || decoded > std::numeric_limits<int>::max()) return false; out = static_cast<int>(decoded); return true; }
    ENCODE() { return out.write(v); }
CODEC_END()

CODEC_BEGIN(std::int8_t)
    DECODE() { std::int64_t decoded; if (!v.read(decoded) || decoded < std::numeric_limits<std::int8_t>::min() || decoded > std::numeric_limits<std::int8_t>::max()) return false; out = static_cast<std::int8_t>(decoded); return true; }
    ENCODE() { return out.write(static_cast<int>(v)); }
CODEC_END()

CODEC_BEGIN(float)
    DECODE() { double decoded; if (!v.read(decoded) || !std::isfinite(decoded) || decoded < -std::numeric_limits<float>::max() || decoded > std::numeric_limits<float>::max()) return false; out = static_cast<float>(decoded); return true; }
    ENCODE() { return out.write(v); }
CODEC_END()

CODEC_BEGIN(std::string)
    DECODE() { std::string_view decoded; if (!v.read(decoded)) return false; out.assign(decoded); return true; }
    ENCODE() { return out.write(v); }
CODEC_END()

CODEC_BEGIN_TPARAMS(typename T, std::optional<T>)
    DECODE() { if (v.isNull()) { out.reset(); return true; } T decoded{}; if (!decodeAny(v, decoded)) return false; out = std::move(decoded); return true; }
    ENCODE() { return v ? encodeAny(out, *v) : out.null(); }
CODEC_END()

CODEC_BEGIN_TPARAMS(typename T, std::vector<T>)
    DECODE() { if (!v.isArray()) return false; CodecType decoded; decoded.reserve(v.size()); for (std::size_t i = 0; i < v.size(); ++i) { T element{}; if (!decodeAny(v.element(i), element)) return false; decoded.push_back(std::move(element)); } out = std::move(decoded); return true; }
    ENCODE() { return out.writeArray([&](json::ArrayWriter&) { for (const auto& element : v) { if (!encodeAny(out, element)) return false; } return true; }); }
CODEC_END()

template<typename T>
inline bool set(json::Reader object, std::string_view key, T& out) {
    const json::Reader value = object.member(key);
    return value.valid() && decodeAny(value, out);
}

} // namespace mg

#include <multigauge/properties/PropertyCodec.h>
