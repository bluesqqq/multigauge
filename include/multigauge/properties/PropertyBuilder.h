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

template <typename T, typename = void>
struct MgPropsParentGetter {
    static constexpr ::PropertyObject::PropertyList::ParentGetter value = nullptr;
};

template <typename T>
struct MgPropsParentGetter<T, std::void_t<decltype(&T::__mg_parent_property_list)>> {
    static constexpr ::PropertyObject::PropertyList::ParentGetter value = &T::__mg_parent_property_list;
};

namespace mg::props {
namespace detail {

template <typename M>
struct MemberPtrTraits;

template <typename C, typename T>
struct MemberPtrTraits<T C::*> {
    using Class = C;
    using Type = T;
};

template <auto MemberPtr>
using MemberClass = typename MemberPtrTraits<decltype(MemberPtr)>::Class;

template <auto MemberPtr>
using MemberType = typename MemberPtrTraits<decltype(MemberPtr)>::Type;

template <auto MemberPtr, auto CallbackPtr>
bool setMember(::PropertyObject* obj, const rapidjson::Value& v) {
    using C = MemberClass<MemberPtr>;
    using T = MemberType<MemberPtr>;
    C* self = static_cast<C*>(obj);

    if constexpr (std::is_base_of_v<::PropertyObject, T>) {
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

template <auto MemberPtr>
bool getMember(const ::PropertyObject* obj, rapidjson::Value& out, rapidjson::Document::AllocatorType& a) {
    using C = MemberClass<MemberPtr>;
    const C* self = static_cast<const C*>(obj);
    return encodeAny(out, a, self->*MemberPtr);
}

template <typename T, typename = void>
struct ChildObjectTraits {
    static constexpr bool supported = false;

    static const ::PropertyObject* getConst(const T&) { return nullptr; }
    static ::PropertyObject* getMutable(T&) { return nullptr; }
};

template <typename T>
struct ChildObjectTraits<T, std::enable_if_t<std::is_base_of_v<::PropertyObject, T>>> {
    static constexpr bool supported = true;

    static const ::PropertyObject* getConst(const T& value) { return &value; }
    static ::PropertyObject* getMutable(T& value) { return &value; }
};

template <typename T>
struct ChildObjectTraits<std::unique_ptr<T>, std::enable_if_t<std::is_base_of_v<::PropertyObject, T>>> {
    static constexpr bool supported = true;

    static const ::PropertyObject* getConst(const std::unique_ptr<T>& value) { return value.get(); }
    static ::PropertyObject* getMutable(std::unique_ptr<T>& value) { return value.get(); }
};

template <typename T>
struct ChildObjectTraits<std::optional<T>, std::enable_if_t<std::is_base_of_v<::PropertyObject, T>>> {
    static constexpr bool supported = true;

    static const ::PropertyObject* getConst(const std::optional<T>& value) {
        return value ? &(*value) : nullptr;
    }

    static ::PropertyObject* getMutable(std::optional<T>& value) {
        return value ? &(*value) : nullptr;
    }
};

template <auto MemberPtr>
const ::PropertyObject* getChildObject(const ::PropertyObject* obj) {
    using C = MemberClass<MemberPtr>;
    using T = MemberType<MemberPtr>;
    static_assert(ChildObjectTraits<T>::supported, "Member must expose a PropertyObject child.");
    const C* self = static_cast<const C*>(obj);
    return ChildObjectTraits<T>::getConst(self->*MemberPtr);
}

template <auto MemberPtr>
::PropertyObject* getChildObjectMutable(::PropertyObject* obj) {
    using C = MemberClass<MemberPtr>;
    using T = MemberType<MemberPtr>;
    static_assert(ChildObjectTraits<T>::supported, "Member must expose a PropertyObject child.");
    C* self = static_cast<C*>(obj);
    return ChildObjectTraits<T>::getMutable(self->*MemberPtr);
}

template <typename T>
rapidjson::Value getEnumOptions(rapidjson::Document::AllocatorType& a) {
    return enumOptionsMeta<EnumTraitsTypeT<T>>(a);
}

template <typename T>
rapidjson::Value getPolymorphicTypes(rapidjson::Document::AllocatorType& a) {
    return MgPolymorphicRegistryTraits<T>::getTypesMeta(a);
}

} // namespace detail

template <auto MemberPtr, auto CallbackPtr = nullptr>
Property makeProperty(const char* key, const char* name, const char* description, PropertyMetadata::RuleListGetter visibleWhen = nullptr, PropertyMetadata::RuleListGetter interactableWhen = nullptr, bool inspectorVisible = true) {
    using T = detail::MemberType<MemberPtr>;

    Property p{};
    p.key = key;
    p.set = &detail::setMember<MemberPtr, CallbackPtr>;
    p.get = &detail::getMember<MemberPtr>;

    PropertyMetadata meta{};
    meta.name = name ? name : key;
    meta.description = description ? description : "No description.";
    meta.widget = MgPropWidgetTraits<T>::value;
    meta.nullable = MgPropNullableTraits<T>::value;
    meta.inspectorVisible = inspectorVisible;
    meta.getVisibleWhen = visibleWhen;
    meta.getInteractableWhen = interactableWhen;
    if constexpr (HasEnumTraitsV<EnumTraitsTypeT<T>>) meta.getOptions = &detail::getEnumOptions<T>;
    if constexpr (MgPolymorphicRegistryTraits<T>::supported) meta.getTypes = &detail::getPolymorphicTypes<T>;

    if constexpr (detail::ChildObjectTraits<T>::supported) {
        p.getChild = &detail::getChildObject<MemberPtr>;
        p.getChildMutable = &detail::getChildObjectMutable<MemberPtr>;
    }

    return {p.key, p.set, p.get, p.getChild, p.getChildMutable, meta};
}

inline Property makeCustomProperty(const char* key, const char* name, const char* description, PropertyMetadata::RuleListGetter visibleWhen, PropertyMetadata::RuleListGetter interactableWhen, bool inspectorVisible, Property::Setter set, Property::Getter get) {
    PropertyMetadata meta{};
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

#define MG_PROPS_PARENT(parent_type) \
    public: \
    static ::PropertyObject::PropertyList __mg_parent_property_list(const ::PropertyObject* __mg_obj) { \
        return static_cast<const parent_type*>(__mg_obj)->parent_type::propertyList(); \
    }

#define MG_PROPS_BEGIN() \
public: \
    ::PropertyObject::PropertyList propertyList() const override { \
        using Self = std::remove_cv_t<std::remove_reference_t<decltype(*this)>>; \
        static const ::Property props[] = {

#define MG_PROP(member, key, display_name, description) \
    ::mg::props::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        true \
    ),

#define MG_PROP_CALLBACK(member, key, display_name, description, callback) \
    ::mg::props::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        true \
    ),

#define MG_PROP_UI(member, key, display_name, description, visible_when, interactable_when) \
    ::mg::props::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        true \
    ),

#define MG_PROP_CALLBACK_UI(member, key, display_name, description, callback, visible_when, interactable_when) \
    ::mg::props::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        true \
    ),

#define MG_PROP_HIDDEN(member, key, display_name, description) \
    ::mg::props::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        false \
    ),

#define MG_PROP_CALLBACK_HIDDEN(member, key, display_name, description, callback) \
    ::mg::props::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        false \
    ),

#define MG_PROP_UI_HIDDEN(member, key, display_name, description, visible_when, interactable_when) \
    ::mg::props::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        false \
    ),

#define MG_PROP_CALLBACK_UI_HIDDEN(member, key, display_name, description, callback, visible_when, interactable_when) \
    ::mg::props::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        false \
    ),

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

#define MG_PROPS_END() \
        }; \
        const auto parentGetter = ::MgPropsParentGetter<Self>::value; \
        return { props, sizeof(props) / sizeof(props[0]), parentGetter }; \
    }
