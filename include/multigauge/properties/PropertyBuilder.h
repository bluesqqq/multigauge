#pragma once

#include <memory>
#include <optional>
#include <concepts>
#include <functional>
#include <type_traits>
#include <utility>

#include <multigauge/json/Json.h>

#include <multigauge/properties/Property.h>
#include <multigauge/properties/PropertyCodec.h>
#include <multigauge/properties/PolymorphicRegistry.h>
#include <multigauge/properties/WidgetTraits.h>

//----------[ PARENT CHAIN ]----------//

namespace mg {

/// Resolves the parent property-list getter exposed by `MG_PROPS_PARENT`.
template <typename T>
constexpr ::mg::PropertyObject::PropertyList::ParentGetter parentPropertyListGetter() {
    if constexpr (requires { &T::__mg_parent_property_list; }) return &T::__mg_parent_property_list;
    return nullptr;
}

} // namespace mg

//----------[ BUILDERS ]----------//

namespace mg::props {

#if MG_ENABLE_EDITOR_REFLECTION
using RuleListGetter = ::mg::PropertyMetadata::RuleListGetter;
#else
using RuleListGetter = bool (*)(::mg::json::Writer&);
#endif

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

/// A data member of a public `PropertyObject` subtype whose value can be
/// serialized by the property system.
template <auto MemberPtr>
concept PropertyMember = std::is_member_object_pointer_v<decltype(MemberPtr)> &&
    requires { typename MemberPtrTraits<decltype(MemberPtr)>::Class; typename MemberPtrTraits<decltype(MemberPtr)>::Type; } &&
    std::derived_from<MemberClass<MemberPtr>, ::mg::PropertyObject>;

/// A no-argument member callback that can be invoked on the member owner.
template <auto CallbackPtr, typename Owner>
concept PropertyCallback = std::is_null_pointer_v<decltype(CallbackPtr)> ||
    requires(Owner& owner) { std::invoke(CallbackPtr, owner); };

//----------[ VALUE ACCESS ]----------//

/// Property setter adapter for a concrete member pointer.
/// @tparam MemberPtr Pointer to the member being assigned.
/// @tparam CallbackPtr Optional callback invoked after assignment.
template <auto MemberPtr, auto CallbackPtr>
    requires PropertyMember<MemberPtr> && PropertyCallback<CallbackPtr, MemberClass<MemberPtr>>
bool setMember(::mg::PropertyObject* obj, json::Reader value) {
    using C = MemberClass<MemberPtr>;
    using T = MemberType<MemberPtr>;
    C* self = static_cast<C*>(obj);

    if constexpr (::mg::PropertyObjectValue<T>) {
    if (!decodeAny<T>(value, self->*MemberPtr)) return false;
    } else {
        T decoded{};
        if (!decodeAny<T>(value, decoded)) return false;
        self->*MemberPtr = std::move(decoded);
    }

    if constexpr (!std::is_same_v<decltype(CallbackPtr), std::nullptr_t>) {
        std::invoke(CallbackPtr, *self);
    }
    return true;
}

/// Property getter adapter for a concrete member pointer.
/// @tparam MemberPtr Pointer to the member being serialized.
template <auto MemberPtr>
    requires PropertyMember<MemberPtr>
bool getMember(const ::mg::PropertyObject* obj, json::Writer& writer) {
    using C = MemberClass<MemberPtr>;
    const C* self = static_cast<const C*>(obj);
    return encodeAny(writer, self->*MemberPtr);
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
template <PropertyObjectValue T>
struct ChildObjectTraits<T> {
    static constexpr bool supported = true;

    static const ::mg::PropertyObject* getConst(const T& value) { return &value; }
    static ::mg::PropertyObject* getMutable(T& value) { return &value; }
};

/// Nested-child support for owned `PropertyObject` pointers.
template <PropertyObjectValue T>
struct ChildObjectTraits<std::unique_ptr<T>> {
    static constexpr bool supported = true;

    static const ::mg::PropertyObject* getConst(const std::unique_ptr<T>& value) { return value.get(); }
    static ::mg::PropertyObject* getMutable(std::unique_ptr<T>& value) { return value.get(); }
};

/// Nested-child support for optional `PropertyObject` members.
template <PropertyObjectValue T>
struct ChildObjectTraits<std::optional<T>> {
    static constexpr bool supported = true;

    static const ::mg::PropertyObject* getConst(const std::optional<T>& value) { return value ? &(*value) : nullptr; }

    static ::mg::PropertyObject* getMutable(std::optional<T>& value) {
        return value ? &(*value) : nullptr;
    }
};

/// Returns the const child object exposed by a member pointer.
template <auto MemberPtr>
    requires PropertyMember<MemberPtr>
const ::mg::PropertyObject* getChildObjectConst(const ::mg::PropertyObject* obj) {
    using C = MemberClass<MemberPtr>;
    using T = MemberType<MemberPtr>;
    static_assert(ChildObjectTraits<T>::supported, "Member must expose a PropertyObject child.");
    const C* self = static_cast<const C*>(obj);
    return ChildObjectTraits<T>::getConst(self->*MemberPtr);
}

/// Returns the mutable child object exposed by a member pointer.
template <auto MemberPtr>
::mg::PropertyObject* getChildObject(::mg::PropertyObject* obj) requires PropertyMember<MemberPtr> {
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
::mg::Property makeProperty(const char* key, const char* name, const char* description, RuleListGetter visibleWhen = nullptr, RuleListGetter interactableWhen = nullptr, bool inspectorVisible = true)
    requires detail::PropertyMember<MemberPtr> && detail::PropertyCallback<CallbackPtr, detail::MemberClass<MemberPtr>> {
    using T = detail::MemberType<MemberPtr>;

    ::mg::Property p{};
    p.key = key;
    p.set = &detail::setMember<MemberPtr, CallbackPtr>;
    p.get = &detail::getMember<MemberPtr>;

    if constexpr (detail::ChildObjectTraits<T>::supported) {
        p.getChild = &detail::getChildObject<MemberPtr>;
        p.getChildConst = &detail::getChildObjectConst<MemberPtr>;
    }

#if MG_ENABLE_EDITOR_REFLECTION
    ::mg::PropertyMetadata meta{};
    meta.name = name ? name : key;
    meta.description = description ? description : "No description.";
    meta.widget = MgPropWidgetTraits<T>::value;
    meta.nullable = MgPropNullableTraits<T>::value;
    meta.inspectorVisible = inspectorVisible;
    meta.getVisibleWhen = visibleWhen;
    meta.getInteractableWhen = interactableWhen;
    if constexpr (::mg::EnumDescribed<::mg::EnumTraitsTypeT<T>>) meta.getOptionsMeta = &::mg::enumOptionsMeta<::mg::EnumTraitsTypeT<T>>;
    if constexpr (::mg::MgPolymorphicRegistryTraits<T>::supported) meta.getTypesMeta = &::mg::MgPolymorphicRegistryTraits<T>::getTypesMeta;

    return {p.key, p.set, p.get, p.getChild, p.getChildConst, meta};
#else
    (void)name; (void)description; (void)visibleWhen; (void)interactableWhen; (void)inspectorVisible;
    return p;
#endif
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
inline ::mg::Property makeCustomProperty(const char* key, const char* name, const char* description, RuleListGetter visibleWhen, RuleListGetter interactableWhen, bool inspectorVisible, ::mg::Property::Setter set, ::mg::Property::Getter get) {
#if MG_ENABLE_EDITOR_REFLECTION
    ::mg::PropertyMetadata meta{};
    meta.name = name ? name : key;
    meta.description = description ? description : "No description.";
    meta.widget = "json";
    meta.inspectorVisible = inspectorVisible;
    meta.getVisibleWhen = visibleWhen;
    meta.getInteractableWhen = interactableWhen;

    return {key, set, get, nullptr, nullptr, meta};
#else
    (void)name; (void)description; (void)visibleWhen; (void)interactableWhen; (void)inspectorVisible;
    return {key, set, get, nullptr, nullptr};
#endif
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

/** Declares a hidden custom property. */
#define MG_PROP_CUSTOM_HIDDEN(key, display_name, description, set_fn, get_fn) \
    ::mg::props::makeCustomProperty( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        false, \
        set_fn, \
        get_fn \
    ),

/** Ends a `propertyList()` override and returns the static property list. */
#define MG_PROPS_END() \
        }; \
        const auto parentGetter = ::mg::parentPropertyListGetter<Self>(); \
        return { props, sizeof(props) / sizeof(props[0]), parentGetter }; \
    }
