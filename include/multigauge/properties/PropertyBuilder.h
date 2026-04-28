#pragma once

#include <memory>
#include <optional>
#include <type_traits>
#include <utility>

#include <rapidjson/document.h>

#include <multigauge/properties/Property.h>
#include <multigauge/properties/PropertyCodec.h>
#include <multigauge/properties/PolymorphicRegistry.h>
#include <multigauge/properties/WidgetTraits.h>

//----------[ PARENT CHAIN ]----------//

namespace mg {

/// Resolves the parent property-list getter exposed by `MG_PROPS_PARENT`.
template <typename T, typename = void>
struct MgPropsParentGetter {
    static constexpr ::mg::PropertyObject::PropertyList::ParentGetter value = nullptr;
};

/// Specialization for types that expose `__mg_parent_property_list`.
template <typename T>
struct MgPropsParentGetter<T, std::void_t<decltype(&T::__mg_parent_property_list)>> {
    static constexpr ::mg::PropertyObject::PropertyList::ParentGetter value = &T::__mg_parent_property_list;
};

} // namespace mg

//----------[ BUILDERS ]----------//

namespace mg::props {

namespace detail {

//----------[ MEMBER TRAITS ]----------//

/// Extracts the owning class and member type from a member pointer.
template <typename M>
struct MemberPtrTraits;

/// Member-pointer specialization used by property builders.
template <typename C, typename T>
struct MemberPtrTraits<T C::*> {
    using Class = C;
    using Type = T;
};

template <auto MemberPtr>
using MemberClass = typename MemberPtrTraits<decltype(MemberPtr)>::Class;

template <auto MemberPtr>
using MemberType = typename MemberPtrTraits<decltype(MemberPtr)>::Type;

//----------[ VALUE ACCESS ]----------//

/// Property setter adapter for a concrete member pointer.
/// @tparam MemberPtr Pointer to the member being assigned.
/// @tparam CallbackPtr Optional callback invoked after assignment.
template <auto MemberPtr, auto CallbackPtr>
bool setMember(::mg::PropertyObject* obj, const rapidjson::Value& v) {
    using C = MemberClass<MemberPtr>;
    using T = MemberType<MemberPtr>;
    C* self = static_cast<C*>(obj);

    if constexpr (std::is_base_of_v<::mg::PropertyObject, T>) {
        if (!decodeAny<T>(v, self->*MemberPtr)) return false;
    } else {
        T decoded{};
        if (!decodeAny<T>(v, decoded)) return false;
        self->*MemberPtr = std::move(decoded);
    }

    if constexpr (!std::is_same_v<decltype(CallbackPtr), std::nullptr_t>) {
        using CbClass = MemberClass<CallbackPtr>;
        auto* cbSelf = static_cast<CbClass*>(obj);
        (cbSelf->*CallbackPtr)();
    }
    return true;
}

/// Property getter adapter for a concrete member pointer.
/// @tparam MemberPtr Pointer to the member being serialized.
template <auto MemberPtr>
bool getMember(const ::mg::PropertyObject* obj, rapidjson::Value& out, rapidjson::Document::AllocatorType& a) {
    using C = MemberClass<MemberPtr>;
    const C* self = static_cast<const C*>(obj);
    return encodeAny(out, a, self->*MemberPtr);
}

//----------[ CHILD OBJECTS ]----------//

/// Detects whether a member exposes a nested `PropertyObject`.
template <typename T, typename = void>
struct ChildObjectTraits {
    static constexpr bool supported = false;

    static const ::mg::PropertyObject* getConst(const T&) { return nullptr; }
    static ::mg::PropertyObject* getMutable(T&) { return nullptr; }
};

/// Nested-child support for direct `PropertyObject` members.
template <typename T>
struct ChildObjectTraits<T, std::enable_if_t<std::is_base_of_v<::mg::PropertyObject, T>>> {
    static constexpr bool supported = true;

    static const ::mg::PropertyObject* getConst(const T& value) { return &value; }
    static ::mg::PropertyObject* getMutable(T& value) { return &value; }
};

/// Nested-child support for owned `PropertyObject` pointers.
template <typename T>
struct ChildObjectTraits<std::unique_ptr<T>, std::enable_if_t<std::is_base_of_v<::mg::PropertyObject, T>>> {
    static constexpr bool supported = true;

    static const ::mg::PropertyObject* getConst(const std::unique_ptr<T>& value) { return value.get(); }
    static ::mg::PropertyObject* getMutable(std::unique_ptr<T>& value) { return value.get(); }
};

/// Nested-child support for optional `PropertyObject` members.
template <typename T>
struct ChildObjectTraits<std::optional<T>, std::enable_if_t<std::is_base_of_v<::mg::PropertyObject, T>>> {
    static constexpr bool supported = true;

    static const ::mg::PropertyObject* getConst(const std::optional<T>& value) { return value ? &(*value) : nullptr; }

    static ::mg::PropertyObject* getMutable(std::optional<T>& value) {
        return value ? &(*value) : nullptr;
    }
};

/// Returns the const child object exposed by a member pointer.
template <auto MemberPtr>
const ::mg::PropertyObject* getChildObjectConst(const ::mg::PropertyObject* obj) {
    using C = MemberClass<MemberPtr>;
    using T = MemberType<MemberPtr>;
    static_assert(ChildObjectTraits<T>::supported, "Member must expose a PropertyObject child.");
    const C* self = static_cast<const C*>(obj);
    return ChildObjectTraits<T>::getConst(self->*MemberPtr);
}

/// Returns the mutable child object exposed by a member pointer.
template <auto MemberPtr>
::mg::PropertyObject* getChildObject(::mg::PropertyObject* obj) {
    using C = MemberClass<MemberPtr>;
    using T = MemberType<MemberPtr>;
    static_assert(ChildObjectTraits<T>::supported, "Member must expose a PropertyObject child.");
    C* self = static_cast<C*>(obj);
    return ChildObjectTraits<T>::getMutable(self->*MemberPtr);
}

} // namespace detail

//----------[ PROPERTY BUILDERS ]----------//

/// Builds a `Property` descriptor for a concrete member pointer.
/// @tparam MemberPtr Pointer to the member exposed as a property.
/// @tparam CallbackPtr Optional callback invoked after successful assignment.
/// @param key Serialized property key.
/// @param name Human-readable display name.
/// @param description Editor-facing description.
/// @param visibleWhen Optional visibility-rule provider for tooling.
/// @param interactableWhen Optional interactability-rule provider for tooling.
/// @param inspectorVisible Whether the property should appear in inspector metadata.
template <auto MemberPtr, auto CallbackPtr = nullptr>
::mg::Property makeProperty(const char* key, const char* name, const char* description, ::mg::PropertyMetadata::RuleListGetter visibleWhen = nullptr, ::mg::PropertyMetadata::RuleListGetter interactableWhen = nullptr, bool inspectorVisible = true) {
    using T = detail::MemberType<MemberPtr>;

    ::mg::Property p{};
    p.key = key;
    p.set = &detail::setMember<MemberPtr, CallbackPtr>;
    p.get = &detail::getMember<MemberPtr>;

    ::mg::PropertyMetadata meta{};
    meta.name = name ? name : key;
    meta.description = description ? description : "No description.";
    meta.widget = MgPropWidgetTraits<T>::value;
    meta.nullable = MgPropNullableTraits<T>::value;
    meta.inspectorVisible = inspectorVisible;
    meta.getVisibleWhen = visibleWhen;
    meta.getInteractableWhen = interactableWhen;
    if constexpr (::mg::HasEnumTraitsV<::mg::EnumTraitsTypeT<T>>) meta.getOptionsMeta = &::mg::enumOptionsMeta<::mg::EnumTraitsTypeT<T>>;
    if constexpr (::mg::MgPolymorphicRegistryTraits<T>::supported) meta.getTypesMeta = &::mg::MgPolymorphicRegistryTraits<T>::getTypesMeta;

    if constexpr (detail::ChildObjectTraits<T>::supported) {
        p.getChild = &detail::getChildObject<MemberPtr>;
        p.getChildConst = &detail::getChildObjectConst<MemberPtr>;
    }

    return {p.key, p.set, p.get, p.getChild, p.getChildConst, meta};
}

/// Builds a `Property` descriptor from custom setter and getter functions.
/// @param key Serialized property key.
/// @param name Human-readable display name.
/// @param description Editor-facing description.
/// @param visibleWhen Optional visibility-rule provider for tooling.
/// @param interactableWhen Optional interactability-rule provider for tooling.
/// @param inspectorVisible Whether the property should appear in inspector metadata.
/// @param set Custom setter function.
/// @param get Custom getter function.
inline ::mg::Property makeCustomProperty(const char* key, const char* name, const char* description, ::mg::PropertyMetadata::RuleListGetter visibleWhen, ::mg::PropertyMetadata::RuleListGetter interactableWhen, bool inspectorVisible, ::mg::Property::Setter set, ::mg::Property::Getter get) {
    ::mg::PropertyMetadata meta{};
    meta.name = name ? name : key;
    meta.description = description ? description : "No description.";
    meta.widget = "json";
    meta.inspectorVisible = inspectorVisible;
    meta.getVisibleWhen = visibleWhen;
    meta.getInteractableWhen = interactableWhen;

    return {key, set, get, nullptr, nullptr, meta};
}

} // namespace mg::props

//----------[ MACROS ]----------//

/** Declares the parent property list for an inherited property hierarchy. */
#define MG_PROPS_PARENT(parent_type) \
    public: \
    static ::mg::PropertyObject::PropertyList __mg_parent_property_list(const ::mg::PropertyObject* __mg_obj) { \
        return static_cast<const parent_type*>(__mg_obj)->parent_type::propertyList(); \
    }

/** Begins a `propertyList()` override backed by a static `Property` array. */
#define MG_PROPS_BEGIN() \
public: \
    ::mg::PropertyObject::PropertyList propertyList() const override { \
        using Self = std::remove_cv_t<std::remove_reference_t<decltype(*this)>>; \
        static const ::mg::Property props[] = {

/** Declares a standard member-backed property. */
#define MG_PROP(member, key, display_name, description) \
    ::mg::props::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        true \
    ),

/** Declares a member-backed property with a post-set callback. */
#define MG_PROP_CALLBACK(member, key, display_name, description, callback) \
    ::mg::props::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        true \
    ),

/** Declares a visible member-backed property with UI rule callbacks. */
#define MG_PROP_UI(member, key, display_name, description, visible_when, interactable_when) \
    ::mg::props::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        true \
    ),

/** Declares a callback-backed property with UI rule callbacks. */
#define MG_PROP_CALLBACK_UI(member, key, display_name, description, callback, visible_when, interactable_when) \
    ::mg::props::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        true \
    ),

/** Declares a hidden member-backed property. */
#define MG_PROP_HIDDEN(member, key, display_name, description) \
    ::mg::props::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        false \
    ),

/** Declares a hidden member-backed property with a post-set callback. */
#define MG_PROP_CALLBACK_HIDDEN(member, key, display_name, description, callback) \
    ::mg::props::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        false \
    ),

/** Declares a hidden member-backed property with UI rule callbacks. */
#define MG_PROP_UI_HIDDEN(member, key, display_name, description, visible_when, interactable_when) \
    ::mg::props::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        false \
    ),

/** Declares a hidden callback-backed property with UI rule callbacks. */
#define MG_PROP_CALLBACK_UI_HIDDEN(member, key, display_name, description, callback, visible_when, interactable_when) \
    ::mg::props::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        false \
    ),

/** Declares a property backed by explicit setter and getter functions. */
#define MG_PROP_CUSTOM(key, display_name, description, set_fn, get_fn) \
    ::mg::props::makeCustomProperty( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        true, \
        set_fn, \
        get_fn \
    ),

/** Declares a custom property with UI rule callbacks. */
#define MG_PROP_CUSTOM_UI(key, display_name, description, visible_when, interactable_when, set_fn, get_fn) \
    ::mg::props::makeCustomProperty( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        true, \
        set_fn, \
        get_fn \
    ),

/** Ends a `propertyList()` override and returns the static property list. */
#define MG_PROPS_END() \
        }; \
        const auto parentGetter = ::mg::MgPropsParentGetter<Self>::value; \
        return { props, sizeof(props) / sizeof(props[0]), parentGetter }; \
    }
