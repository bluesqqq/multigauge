#pragma once

#include <rapidjson/document.h>

#include <cstdint>
#include <cmath>
#include <limits>
#include <string>
#include <optional>
#include <type_traits>
#include <utility>
#include <vector>

namespace mg {

class PropertyObject;

template<typename T>
struct Codec;

// ---- HasCodec trait ----
template <typename T, typename = void>
struct HasCodec : std::false_type {};

template <typename T>
struct HasCodec<T, std::void_t<
    decltype(Codec<T>::decode(std::declval<const rapidjson::Value&>(), std::declval<T&>())),
    decltype(Codec<T>::encode(std::declval<rapidjson::Value&>(), std::declval<rapidjson::Document::AllocatorType&>(), std::declval<const T&>()))
>> : std::true_type {};

template <typename T>
inline constexpr bool HasCodecV = HasCodec<T>::value;

template <typename T>
bool decodeAny(const rapidjson::Value& v, T& out);

template <typename T>
bool encodeAny(rapidjson::Value& out, rapidjson::Document::AllocatorType& a, const T& v);

//----------[ MACROS ]----------//

template <class T>
struct mg_remove_cvref {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

#define CODEC_FRIEND(T) friend struct Codec<T>;

// Declaration

#define CODEC_BEGIN(CODEC_T) \
    template<> struct Codec<CODEC_T> { using CodecType = CODEC_T;

#define CODEC_BEGIN_TPARAMS(TPARAMS, CODEC_T) \
    template<TPARAMS> struct Codec<CODEC_T> { using CodecType = CODEC_T;

#define DECODE() \
    static bool decode(const rapidjson::Value& v, CodecType& out)

#define ENCODE() \
    static bool encode(rapidjson::Value& out, rapidjson::Document::AllocatorType& a, const CodecType& v)

#define CODEC_END() \
    };

// Implementation

#define DECODE_IMPL(CODEC_T) \
    bool Codec<CODEC_T>::decode(const rapidjson::Value& v, CodecType& out)

#define ENCODE_IMPL(CODEC_T) \
    bool Codec<CODEC_T>::encode(rapidjson::Value& out, rapidjson::Document::AllocatorType& a, const CodecType& v)

//----------[ PRIMITIVE TYPES ]----------//

CODEC_BEGIN(bool)
    DECODE() {
        if (!v.IsBool()) return false;
        out = v.GetBool();
        return true;
    }

    ENCODE() {
        out.SetBool(v);
        return true;
    }
CODEC_END()

CODEC_BEGIN(int)
    DECODE() {
        if (!v.IsInt()) return false;
        out = v.GetInt();
        return true;
    }

    ENCODE() {
        out.SetInt(v);
        return true;
    }
CODEC_END()

CODEC_BEGIN(std::int8_t)
    DECODE() {
        if (!v.IsInt()) return false;
        const int value = v.GetInt();
        if (value < std::numeric_limits<std::int8_t>::min() || value > std::numeric_limits<std::int8_t>::max()) {
            return false;
        }

        out = static_cast<std::int8_t>(value);
        return true;
    }

    ENCODE() {
        out.SetInt(static_cast<int>(v));
        return true;
    }
CODEC_END()

CODEC_BEGIN(float)
    DECODE() {
        if (!v.IsNumber()) return false;
        const double value = v.GetDouble();
        if (!std::isfinite(value) || value < -std::numeric_limits<float>::max() || value > std::numeric_limits<float>::max()) return false;
        out = static_cast<float>(value);
        return true;
    }

    ENCODE() {
        out.SetFloat(v);
        return true;
    }
CODEC_END()

CODEC_BEGIN(std::string)
    DECODE() {
        if (!v.IsString()) return false;
        out.assign(v.GetString(), v.GetStringLength());
        return true;
    }

    ENCODE() {
        out.SetString(v.c_str(), static_cast<rapidjson::SizeType>(v.size()), a);
        return true;
    }
CODEC_END()

CODEC_BEGIN(const char*)
    DECODE() {
        if (!v.IsString()) return false;
        out = v.GetString();
        return true;
    }

    ENCODE() {
        if (!v) { out.SetNull(); return true; }
        out.SetString(v, a);
        return true;
    }
CODEC_END()

CODEC_BEGIN_TPARAMS(typename T, std::optional<T>)
    DECODE() {
        if (v.IsNull()) {
            out.reset();
            return true;
        }

        T tmp{};
        if (!decodeAny(v, tmp)) return false;
        out = std::move(tmp);
        return true;
    }
    
    ENCODE() {
        if (!v) { out.SetNull(); return true; }
        return encodeAny(out, a, *v);
    }
CODEC_END()

CODEC_BEGIN_TPARAMS(typename T, std::vector<T>)
    DECODE() {
        if (!v.IsArray()) return false;

        CodecType tmp;
        tmp.reserve(v.Size());

        for (auto it = v.Begin(); it != v.End(); ++it) {
            T elem{};
            if (!decodeAny(*it, elem)) return false;
            tmp.push_back(std::move(elem));
        }

        out = std::move(tmp);
        return true;
    }

    ENCODE() {
        out.SetArray();
        out.Reserve(static_cast<rapidjson::SizeType>(v.size()), a);

        for (const auto& e : v) {
            rapidjson::Value elem;
            if (!encodeAny(elem, a, e)) return false;
            out.PushBack(elem, a);
        }

        return true;
    }
CODEC_END()

// Convenience: read key from object and decode using decodeAny
template<typename T>
inline bool set(const rapidjson::Value::ConstObject& o, const char* key, T& out) {
    auto it = o.FindMember(key);
    if (it == o.MemberEnd()) return false;
    return decodeAny(it->value, out);
}

} // namespace mg

#include <multigauge/properties/PropertyCodec.h>
