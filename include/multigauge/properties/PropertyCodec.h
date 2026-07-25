#pragma once

#include <multigauge/properties/Codec.h>

namespace mg {

class PropertyObject;
template <typename T, typename = void> struct PolymorphicOwnedTraits { static constexpr bool supported = false; };

template <typename Base, typename Owned>
inline bool decodePolymorphicOwned(json::Reader value, Owned& out) {
    if (value.isNull()) { out = nullptr; return true; }
    if (!value.isObject()) return false;
    std::string_view typeView;
    (void)value.member("type").read(typeView);
    Owned decoded = Base::registry().create(typeView);
    if (!decoded || !decoded->loadProperties(value)) return false;
    out = std::move(decoded);
    return true;
}

template <typename Owned>
inline bool encodePolymorphicOwned(json::Writer& writer, const Owned& value) {
    return value ? value->saveProperties(writer) : writer.null();
}

template <typename T>
inline bool decodeAny(json::Reader value, T& out) {
    if constexpr (HasCodecV<T>) { if (Codec<T>::decode(value, out)) return true; }
    if constexpr (PolymorphicOwnedTraits<T>::supported) { using Base = typename PolymorphicOwnedTraits<T>::Base; if (decodePolymorphicOwned<Base>(value, out)) return true; }
    if constexpr (std::is_base_of_v<PropertyObject, T>) { return value.isObject() && out.loadProperties(value); }
    static_assert(HasCodecV<T> || std::is_base_of_v<PropertyObject, T> || PolymorphicOwnedTraits<T>::supported, "Type must have Codec<T>, be a PropertyObject, or use the polymorphic owned path.");
    return false;
}

template <typename T>
inline bool encodeAny(json::Writer& writer, const T& value) {
    if constexpr (HasCodecV<T>) { if (Codec<T>::encode(writer, value)) return true; }
    if constexpr (PolymorphicOwnedTraits<T>::supported) { if (encodePolymorphicOwned(writer, value)) return true; }
    if constexpr (std::is_base_of_v<PropertyObject, T>) return value.saveProperties(writer);
    static_assert(HasCodecV<T> || std::is_base_of_v<PropertyObject, T> || PolymorphicOwnedTraits<T>::supported, "Type must have Codec<T>, be a PropertyObject, or use the polymorphic owned path.");
    return false;
}

} // namespace mg
