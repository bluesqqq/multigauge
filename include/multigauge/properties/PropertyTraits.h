#pragma once

#include <concepts>
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

//----------[ COLLECTION ]----------//

/// Describes a collection whose items can be edited through inspector metadata.
///
/// Specialize this trait for collection value types that need to normalize or
/// validate mutations beyond the default vector behavior.
template <typename T>
struct InspectorCollectionTraits { static constexpr bool supported = false; };

template <PropertyObjectValue Item>
struct InspectorCollectionTraits<std::vector<Item>> {
    static constexpr bool supported = true;
    using ItemType = Item;

    static bool normalize(std::vector<Item>&) { return true; }

    static bool writeDefault(const std::vector<Item>&, json::Writer& writer) {
        return encodeAny(writer, Item{});
    }
};

template <typename Base>
    requires std::derived_from<Base, ::mg::PropertyObject> &&
             ::mg::MgPolymorphicRegistryTraits<std::unique_ptr<Base>>::supported
struct InspectorCollectionTraits<std::vector<std::unique_ptr<Base>>> {
    static constexpr bool supported = true;
    using Owned = std::unique_ptr<Base>;
    using ItemType = Base;

    static bool getTypesMeta(json::Writer& writer) {
        return ::mg::MgPolymorphicRegistryTraits<Owned>::getTypesMeta(writer);
    }

    static bool normalize(std::vector<Owned>&) { return true; }
};

} // namespace props::detail

} // namespace mg
