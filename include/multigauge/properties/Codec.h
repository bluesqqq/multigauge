#pragma once

#include <multigauge/json/Json.h>

#include <cmath>
#include <concepts>
#include <cstdint>
#include <limits>
#include <optional>
#include <string>
#include <utility>
#include <vector>

namespace mg {

class PropertyObject; // Forward declaration


/// @brief Serialization adapter for a type used by the property system.
/// @tparam T Type serialized by the codec.
template<typename T>
struct Codec;

/// @brief Determines whether a type provides a valid property codec.
/// @tparam T Type to test.
template <typename T>
concept CodecFor = requires(json::Reader reader, json::Writer& writer, T& out, const T& value) {
    { Codec<T>::decode(reader, out) } -> std::same_as<bool>;
    { Codec<T>::encode(writer, value) } -> std::same_as<bool>;
};

/// @brief Decodes a value through the property serialization system.
/// @tparam T Destination type.
/// @param value JSON value to decode.
/// @param out Destination receiving the decoded value.
/// @return true if decoding succeeds; otherwise false.
template <typename T> bool decodeAny(
    json::Reader value,
    T& out
);

/// @brief Encodes a value through the property serialization system.
/// @tparam T Value type.
/// @param writer JSON writer receiving the encoded value.
/// @param value Value to encode.
/// @return true if encoding succeeds; otherwise false.
template <typename T> bool encodeAny(
    json::Writer& writer,
    const T& value
);

//----------[ MACROS ]----------//

/// @brief Grants Codec<T> access to private members of a type.
#define CODEC_FRIEND(T) friend struct Codec<T>;

/// @brief Begins a full specialization of Codec for a concrete type.
#define CODEC_BEGIN(CODEC_T) template <> struct Codec<CODEC_T> { using CodecType = CODEC_T;

/// @brief Begins a templated Codec specialization.
#define CODEC_BEGIN_TPARAMS(TPARAMS, CODEC_T) template <TPARAMS> struct Codec<CODEC_T> { using CodecType = CODEC_T;

/// @brief Ends a Codec specialization declared with CODEC_BEGIN.
#define CODEC_END() };

/// @brief Declares the standard decode function for a Codec specialization.
#define DECODE() static bool decode(::mg::json::Reader v, CodecType& out)

/// @brief Declares the standard encode function for a Codec specialization.
#define ENCODE() static bool encode(::mg::json::Writer& out, const CodecType& v)

/// @brief Defines an out-of-line Codec decode function.
#define DECODE_IMPL(CODEC_T) bool Codec<CODEC_T>::decode(::mg::json::Reader v, CODEC_T& out)

/// @brief Defines an out-of-line Codec encode function.
#define ENCODE_IMPL(CODEC_T) bool Codec<CODEC_T>::encode(::mg::json::Writer& out, const CODEC_T& v)

} // namespace mg

#include <multigauge/properties/PropertyCodec.h>
#include <multigauge/properties/codecs/StandardCodecs.h>
