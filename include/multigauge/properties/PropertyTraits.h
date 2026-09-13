#pragma once

#include <concepts>
#include <functional>
#include <memory>
#include <optional>
#include <type_traits>
#include <vector>

#include <multigauge/properties/PolymorphicRegistry.h>
#include <multigauge/properties/PropertyCodec.h>
#include <multigauge/properties/Property.h>

namespace mg {

//----------[ NULLABLE ]----------//

/// Declares whether a property value type accepts JSON null.
template <typename T>
struct PropertyNullableTraits { static constexpr bool value = false; };

template <typename T>
struct PropertyNullableTraits<std::optional<T>> { static constexpr bool value = true; };

namespace props::detail {

//----------[ MEMBER ]----------//

template <typename M>
struct MemberPtrTraits;

template <typename C, typename T>
struct MemberPtrTraits<T C::*> { using Class = C; using Type = T; };

template <auto MemberPtr>
using MemberClass = typename MemberPtrTraits<decltype(MemberPtr)>::Class;

template <auto MemberPtr>
using MemberType = typename MemberPtrTraits<decltype(MemberPtr)>::Type;

template <auto MemberPtr>
concept PropertyMember =
    std::is_member_object_pointer_v<decltype(MemberPtr)> &&
    requires {
        typename MemberPtrTraits<decltype(MemberPtr)>::Class;
        typename MemberPtrTraits<decltype(MemberPtr)>::Type;
    } &&
    std::derived_from<MemberClass<MemberPtr>, ::mg::PropertyObject>;

template <auto CallbackPtr, typename Owner>
concept PropertyCallback =
    std::is_null_pointer_v<decltype(CallbackPtr)> ||
    requires(Owner& owner) {
        std::invoke(CallbackPtr, owner);
    };

//----------[ CHILD ]----------//

/// Detects whether a property value exposes a nested `PropertyObject`.
template <typename T>
struct ChildObjectTraits {
    static constexpr bool supported = false;
    static const ::mg::PropertyObject* getConst(const T&) { return nullptr; }
};

template <PropertyObjectValue T>
struct ChildObjectTraits<T> {
    static constexpr bool supported = true;
    static const ::mg::PropertyObject* getConst(const T& value) { return &value; }
};

template <PropertyObjectValue T>
struct ChildObjectTraits<std::optional<T>> {
    static constexpr bool supported = true;
    static const ::mg::PropertyObject* getConst(const std::optional<T>& value) { return value ? &(*value) : nullptr; }
};

template <PropertyObjectValue T>
struct ChildObjectTraits<std::unique_ptr<T>> {
    static constexpr bool supported = true;
    static const ::mg::PropertyObject* getConst(const std::unique_ptr<T>& value) { return value.get(); }
};

//----------[ POLYMORPHIC COLLECTION ]----------//

/// Detects a vector of owned polymorphic `PropertyObject`s.
template <typename T>
struct PolymorphicCollectionTraits { static constexpr bool supported = false; };

template <typename Base>
    requires std::derived_from<Base, ::mg::PropertyObject> &&
             ::mg::MgPolymorphicRegistryTraits<std::unique_ptr<Base>>::supported
struct PolymorphicCollectionTraits<std::vector<std::unique_ptr<Base>>> {
    static constexpr bool supported = true;
    using Owned = std::unique_ptr<Base>;

    static bool getTypesMeta(json::Writer& writer) {
        return ::mg::MgPolymorphicRegistryTraits<Owned>::getTypesMeta(writer);
    }
};

} // namespace props::detail

} // namespace mg
