#pragma once

#include <multigauge/properties/Codec.h>
#include <multigauge/utils/Json.h>

#include <string_view>

namespace mg {

class PropertyObject;

template <typename T, typename = void>
struct PolymorphicOwnedTraits {
    static constexpr bool supported = false;
};

template <typename Base, typename Owned>
inline bool decodePolymorphicOwned(const rapidjson::Value& v, Owned& out) {
    if (v.IsNull()) {
        out = nullptr;
        return true;
    }

    if (!v.IsObject()) return false;

    const auto obj = v.GetObject();

    std::string_view type;
    const auto typeMember = v.FindMember("type");
    if (typeMember != v.MemberEnd() && typeMember->value.IsString()) {
        type = std::string_view(typeMember->value.GetString(), typeMember->value.GetStringLength());
    }

    Owned decoded = Base::registry().create(type);
    if (!decoded) return false;

    if (!decoded->loadProperties(obj)) return false;
    out = std::move(decoded);
    return true;
}

template <typename Owned>
inline bool encodePolymorphicOwned(rapidjson::Value& out, rapidjson::Document::AllocatorType& a, const Owned& v) {
    if (!v) {
        out.SetNull();
        return true;
    }

    v->saveProperties(out, a);
    return true;
}

template <typename T>
inline bool decodeAny(const rapidjson::Value& v, T& out) {
    // Codec<T>
    if constexpr (HasCodecV<T>) {
        if (Codec<T>::decode(v, out)) return true;
    }

    // Registry-backed polymorphic owned type
    if constexpr (PolymorphicOwnedTraits<T>::supported) {
        using Base = typename PolymorphicOwnedTraits<T>::Base;
        if (decodePolymorphicOwned<Base>(v, out)) return true;
    }

    // PropertyObject-derived type
    if constexpr (std::is_base_of_v<PropertyObject, T>) {
        if (!v.IsObject()) return false;
        return out.loadProperties(v.GetObject());
    }

    static_assert(HasCodecV<T> || std::is_base_of_v<PropertyObject, T> || PolymorphicOwnedTraits<T>::supported, "Type must have Codec<T>, be a PropertyObject, or use the polymorphic owned path.");
    return false;
}

template <typename T>
inline bool encodeAny(rapidjson::Value& out, rapidjson::Document::AllocatorType& a, const T& v) {
    // Codec<T>
    if constexpr (HasCodecV<T>) {
        if (Codec<T>::encode(out, a, v)) return true;
    }

    // Registry-backed polymorphic owned type
    if constexpr (PolymorphicOwnedTraits<T>::supported) {
        if (encodePolymorphicOwned(out, a, v)) return true;
    }

    // PropertyObject-derived type
    if constexpr (std::is_base_of_v<PropertyObject, T>) {
        v.saveProperties(out, a);
        return true;
    }

    static_assert(HasCodecV<T> || std::is_base_of_v<PropertyObject, T> || PolymorphicOwnedTraits<T>::supported, "Type must have Codec<T> with encode/decode, be a PropertyObject, or use the polymorphic owned path.");
    return false;
}

} // namespace mg
