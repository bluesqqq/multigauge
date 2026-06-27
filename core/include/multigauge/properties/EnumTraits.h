#pragma once

#include <cstring>
#include <optional>
#include <rapidjson/document.h>
#include <type_traits>

namespace mg {

template <typename T>
struct EnumOption {
    T value;
    const char* key;
    const char* label;
};

template <typename T>
struct EnumTraits;

template <typename T, typename = void>
struct HasEnumTraits : std::false_type {};

template <typename T>
struct HasEnumTraits<T, std::void_t<decltype(EnumTraits<T>::options)>> : std::true_type {};

template <typename T>
inline constexpr bool HasEnumTraitsV = HasEnumTraits<T>::value;

template <typename T>
struct EnumTraitsType {
    using type = std::remove_cv_t<std::remove_reference_t<T>>;
};

template <typename T>
struct EnumTraitsType<std::optional<T>> {
    using type = typename EnumTraitsType<T>::type;
};

template <typename T>
using EnumTraitsTypeT = typename EnumTraitsType<T>::type;

template <typename E>
bool decodeEnum(const rapidjson::Value& v, E& out) {
    if (!v.IsString()) return false;
    const char* s = v.GetString();
    if (!s) return false;

    for (const auto& opt : EnumTraits<E>::options) {
        if (std::strcmp(opt.key, s) == 0) {
            out = opt.value;
            return true;
        }
    }
    return false;
}

template <typename E>
bool encodeEnum(rapidjson::Value& out, rapidjson::Document::AllocatorType& a, E value) {
    for (const auto& opt : EnumTraits<E>::options) {
        if (opt.value == value) {
            out.SetString(opt.key, a);
            return true;
        }
    }
    return false;
}

template <typename E>
rapidjson::Value enumOptionsMeta(rapidjson::Document::AllocatorType& a) {
    rapidjson::Value options(rapidjson::kArrayType);

    for (const auto& opt : EnumTraits<E>::options) {
        rapidjson::Value entry(rapidjson::kObjectType);
        entry.AddMember("value", rapidjson::Value(opt.key, a), a);
        entry.AddMember("label", rapidjson::Value(opt.label ? opt.label : opt.key, a), a);
        options.PushBack(entry, a);
    }

    return options;
}

} // namespace mg
