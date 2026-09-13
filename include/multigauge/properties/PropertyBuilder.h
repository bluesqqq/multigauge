#pragma once

#include <utility>

#include <multigauge/properties/PropertyTraits.h>

namespace mg::props {
namespace detail {

template <typename T>
constexpr ::mg::PropertyObject::PropertyList::ParentGetter parentPropertyListGetter() {
    if constexpr (requires { &T::__mg_parent_property_list; }) return &T::__mg_parent_property_list;
    return nullptr;
}

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

    if constexpr (!std::is_same_v<decltype(CallbackPtr), std::nullptr_t>) std::invoke(CallbackPtr, *self);
    return true;
}

template <auto MemberPtr>
    requires PropertyMember<MemberPtr>
bool getMember(const ::mg::PropertyObject* obj, json::Writer& writer) {
    using C = MemberClass<MemberPtr>;
    return encodeAny(writer, static_cast<const C*>(obj)->*MemberPtr);
}

template <auto MemberPtr>
    requires PropertyMember<MemberPtr>
const ::mg::PropertyObject* getChildObject(const ::mg::PropertyObject* obj) {
    using C = MemberClass<MemberPtr>;
    using T = MemberType<MemberPtr>;

    static_assert(ChildObjectTraits<T>::supported, "Member must expose a PropertyObject child.");

    return ChildObjectTraits<T>::getConst(static_cast<const C*>(obj)->*MemberPtr);
}

template <auto MemberPtr>
    requires PropertyMember<MemberPtr> && PolymorphicCollectionTraits<MemberType<MemberPtr>>::supported
bool writeCollectionItems(const ::mg::PropertyObject* obj, json::Writer& writer) {
    using C = MemberClass<MemberPtr>;
    
    const C* self = static_cast<const C*>(obj);

    return writer.writeArray([&](json::ArrayWriter& items) {
        for (const auto& item : self->*MemberPtr) {
            if (!items.writeObject([&](json::ObjectWriter& entry) {
                if (!item) return entry.writeValue("type", [](json::Writer& value) { return value.null(); });
                if (!entry.write("type", item->typeId() ? item->typeId() : "")) return false;
                return entry.writeValue("inspector", [&](json::Writer& inspector) {
                    return item->writeInspectorMeta(inspector);
                });
            })) return false;
        }
        return true;
    });
}

} // namespace detail

template <auto MemberPtr, auto CallbackPtr = nullptr>
::mg::Property makeProperty(const char* key)
    requires detail::PropertyMember<MemberPtr> && detail::PropertyCallback<CallbackPtr, detail::MemberClass<MemberPtr>> {
    using T = detail::MemberType<MemberPtr>;
    ::mg::Property p{key, &detail::setMember<MemberPtr, CallbackPtr>, &detail::getMember<MemberPtr>, nullptr};
    if constexpr (detail::ChildObjectTraits<T>::supported) p.getChild = &detail::getChildObject<MemberPtr>;
#if MG_BUILD_EDITOR
    ::mg::PropertyMetadata meta{};
    meta.nullable = PropertyNullableTraits<T>::value;
    if constexpr (::mg::EnumDescribed<::mg::EnumTraitsTypeT<T>>) meta.getOptions = &::mg::enumOptionsMeta<::mg::EnumTraitsTypeT<T>>;
    if constexpr (::mg::MgPolymorphicRegistryTraits<T>::supported) meta.getTypes = &::mg::MgPolymorphicRegistryTraits<T>::getTypesMeta;
    if constexpr (detail::PolymorphicCollectionTraits<T>::supported) {
        meta.getTypes = &detail::PolymorphicCollectionTraits<T>::getTypesMeta;
        meta.getCollectionItems = &detail::writeCollectionItems<MemberPtr>;
    }
    return {p.key, p.set, p.get, p.getChild, meta};
#else
    return p;
#endif
}

inline ::mg::Property makeCustomProperty(const char* key, ::mg::Property::Setter set, ::mg::Property::Getter get) {
#if MG_BUILD_EDITOR
    ::mg::PropertyMetadata meta{};
    return {key, set, get, nullptr, meta};
#else
    return {key, set, get, nullptr};
#endif
}

} // namespace mg::props

//----------[ MACROS ]----------//

#define MG_PROPS_PARENT(parent_type) \
    public: \
    static ::mg::PropertyObject::PropertyList __mg_parent_property_list(const ::mg::PropertyObject* __mg_obj) { \
        return static_cast<const parent_type*>(__mg_obj)->parent_type::propertyList(); \
    }

#define MG_PROPS_BEGIN() \
public: \
    ::mg::PropertyObject::PropertyList propertyList() const override { \
        using Self = std::remove_cv_t<std::remove_reference_t<decltype(*this)>>; \
        static const ::mg::Property props[] = {

#define MG_PROP(member, key) ::mg::props::makeProperty<&Self::member, nullptr>(key),

#define MG_PROP_CALLBACK(member, key, callback) ::mg::props::makeProperty<&Self::member, callback>(key),

#define MG_PROP_CUSTOM(key, set_fn, get_fn) ::mg::props::makeCustomProperty(key, set_fn, get_fn),

#define MG_PROPS_END() \
        }; \
        const auto parentGetter = ::mg::props::detail::parentPropertyListGetter<Self>(); \
        return { props, sizeof(props) / sizeof(props[0]), parentGetter }; \
    }
