#pragma once

#include <multigauge/json/Json.h>

#include <concepts>
#include <optional>
#include <type_traits>

namespace mg {
template <typename T> struct EnumOption { T value; const char* key; const char* label; };
template <typename T> struct EnumTraits;
template <typename T>
concept EnumDescribed = requires { EnumTraits<T>::options; };
template <typename T> struct EnumTraitsType { using type = std::remove_cv_t<std::remove_reference_t<T>>; };
template <typename T> struct EnumTraitsType<std::optional<T>> { using type = typename EnumTraitsType<T>::type; };
template <typename T> using EnumTraitsTypeT = typename EnumTraitsType<T>::type;

template <typename E> bool decodeEnum(json::Reader value, E& out) {
    std::string_view key; if (!value.read(key)) return false;
    for (const auto& option : EnumTraits<E>::options) if (key == option.key) { out = option.value; return true; }
    return false;
}
template <typename E> bool encodeEnum(json::Writer& writer, E value) {
    for (const auto& option : EnumTraits<E>::options) if (option.value == value) return writer.write(option.key);
    return false;
}
template <typename E> bool enumOptionsMeta(json::Writer& writer) {
    return writer.writeArray([&](json::ArrayWriter& options) {
        for (const auto& option : EnumTraits<E>::options) if (!options.writeObject([&](json::ObjectWriter& entry) { return entry.write("value", option.key) && entry.write("label", option.label ? option.label : option.key); })) return false;
        return true;
    });
}
} // namespace mg
