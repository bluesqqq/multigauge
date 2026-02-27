#pragma once

#include <rapidjson/document.h>

#include <string>
#include <optional>
#include <type_traits>
#include <utility>   // std::move, std::declval

// Forward declaration to avoid include cycles.
// Full definition is provided by Editable.h.
class Editable;

template<typename T>
struct Codec;

// ---- HasCodec trait ----
template <typename T, typename = void>
struct HasCodec : std::false_type {};

template <typename T>
struct HasCodec<T, std::void_t<
    decltype(Codec<T>::decode(std::declval<const rapidjson::Value&>(), std::declval<T&>()))
>> : std::true_type {};

template <typename T>
inline constexpr bool HasCodecV = HasCodec<T>::value;

// ---- decodeAny declaration (definition is in Editable.h, after Editable is complete) ----
template <typename T>
bool decodeAny(const rapidjson::Value& v, T& out);

//----------[ PRIMITIVE TYPES ]----------//

// bool
template<>
struct Codec<bool> {
    static bool decode(const rapidjson::Value& v, bool& out) {
        if (!v.IsBool()) return false;
        out = v.GetBool();
        return true;
    }
};

// int
template<>
struct Codec<int> {
    static bool decode(const rapidjson::Value& v, int& out) {
        if (!v.IsNumber()) return false;
        out = v.GetInt();
        return true;
    }
};

// float
template<>
struct Codec<float> {
    static bool decode(const rapidjson::Value& v, float& out) {
        if (!v.IsNumber()) return false;
        out = v.GetFloat();
        return true;
    }
};

// std::string
template<>
struct Codec<std::string> {
    static bool decode(const rapidjson::Value& v, std::string& out) {
        if (!v.IsString()) return false;
        out.assign(v.GetString(), v.GetStringLength());
        return true;
    }
};

// const char*
template<>
struct Codec<const char*> {
    static bool decode(const rapidjson::Value& v, const char*& out) {
        if (!v.IsString()) return false;
        out = v.GetString();
        return true;
    }
};

// std::optional<T>
// NOTE: uses decodeAny so optional<EditableDerived> works without Codec<T>.
template<typename T>
struct Codec<std::optional<T>> {
    static bool decode(const rapidjson::Value& v, std::optional<T>& out) {
        if (v.IsNull()) {
            out.reset();
            return true;
        }

        T tmp{};
        if (!decodeAny(v, tmp)) return false;
        out = std::move(tmp);
        return true;
    }
};

// Convenience: read key from object and decode using decodeAny
template<typename T>
inline bool set(const rapidjson::Value::ConstObject& o, const char* key, T& out) {
    auto it = o.FindMember(key);
    if (it == o.MemberEnd()) return false;
    return decodeAny(it->value, out);
}