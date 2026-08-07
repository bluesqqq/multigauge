#pragma once

#include <multigauge/properties/Codec.h>

#include <type_traits>

namespace mg {

class PropertyObject;

/// @brief A publicly derived PropertyObject suitable for inline property storage.
template <typename T>
concept PropertyObjectValue =
    std::derived_from<std::remove_cv_t<T>, PropertyObject>;

/// @brief Decodes an owning polymorphic PropertyObject using the base type registry.
/// @tparam Base Base PropertyObject type providing the registry.
/// @tparam Owned Owning pointer-like type receiving the decoded object.
/// @param value JSON value to decode.
/// @param out Destination receiving the decoded object, or null for a JSON null value.
/// @return true if decoding succeeds; otherwise false.
template <typename Base, typename Owned>
    requires std::derived_from<Base, PropertyObject>
inline bool decodePolymorphicOwned(json::Reader value, Owned& out) {
    if (value.isNull()) {
        out = nullptr;
        return true;
    }

    if (!value.isObject()) return false;

    std::string_view typeView;
    (void)value.member("type").read(typeView);

    Owned decoded = Base::registry().create(typeView);
    if (!decoded || !decoded->loadProperties(value)) return false;

    out = std::move(decoded);
    return true;
}

/// @brief Encodes an owning polymorphic PropertyObject.
/// @tparam Owned Owning pointer-like type containing the object to encode.
/// @param writer JSON writer receiving the encoded value.
/// @param value Object to encode, or an empty owner to encode as JSON null.
/// @return true if encoding succeeds; otherwise false.
template <typename Owned>
inline bool encodePolymorphicOwned(json::Writer& writer, const Owned& value) {
    return value ? value->saveProperties(writer) : writer.null();
}

/// @brief Decodes a value using its Codec or PropertyObject serialization.
/// @tparam T Type to decode.
/// @param value JSON value to decode.
/// @param out Destination receiving the decoded value.
/// @return true if decoding succeeds; otherwise false.
template <typename T>
inline bool decodeAny(json::Reader value, T& out) {
    static_assert(
        CodecFor<T> || PropertyObjectValue<T>,
        "Type must have a valid Codec<T> or be a publicly derived PropertyObject."
    );

    if constexpr (CodecFor<T>) {
        if (Codec<T>::decode(value, out)) return true;
    }

    if constexpr (PropertyObjectValue<T>) return value.isObject() && out.loadProperties(value);

    return false;
}

/// @brief Encodes a value using its Codec or PropertyObject serialization.
/// @tparam T Type to encode.
/// @param writer JSON writer receiving the encoded value.
/// @param value Value to encode.
/// @return true if encoding succeeds; otherwise false.
template <typename T>
inline bool encodeAny(json::Writer& writer, const T& value) {
    static_assert(
        CodecFor<T> || PropertyObjectValue<T>,
        "Type must have a valid Codec<T> or be a publicly derived PropertyObject."
    );

    if constexpr (CodecFor<T>) {
        if (Codec<T>::encode(writer, value)) return true;
    }

    if constexpr (PropertyObjectValue<T>) return value.saveProperties(writer);

    return false;
}

} // namespace mg
