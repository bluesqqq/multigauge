#pragma once

#include <multigauge/properties/Codec.h>

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

    const char* type = nullptr;
    if (auto it = obj.FindMember("type"); it != obj.MemberEnd() && it->value.IsString()) {
        type = it->value.GetString();
    }

    out = Base::registry().create(type);
    if (!out) return false;

    out->loadProperties(obj);
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
        out.loadProperties(v.GetObject());
        return true;
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
