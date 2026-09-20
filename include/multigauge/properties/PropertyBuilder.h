#pragma once

#include <cstddef>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include <multigauge/properties/PropertyTraits.h>

namespace mg::props {
namespace detail {

template <typename T>
constexpr ::mg::PropertyObject::PropertyList::ParentGetter parentPropertyListGetter() {
    if constexpr (requires { &T::__mg_parent_property_list; }) return &T::__mg_parent_property_list;
    return nullptr;
}

template <auto MemberPtr>
    requires PropertyMember<MemberPtr>
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

    return true;
}

template <auto MemberPtr>
    requires PropertyMember<MemberPtr>
bool getMember(const ::mg::PropertyObject* obj, json::Writer& writer) {
    using C = MemberClass<MemberPtr>;
    return encodeAny(writer, static_cast<const C*>(obj)->*MemberPtr);
}

#if MG_BUILD_EDITOR
template <auto MemberPtr>
    requires PropertyMember<MemberPtr>
bool validateMember(const ::mg::PropertyObject*, json::Reader value) {
    using T = MemberType<MemberPtr>;

    T decoded{};
    return decodeAny<T>(value, decoded);
}
#endif

template <auto MemberPtr>
    requires PropertyMember<MemberPtr>
const ::mg::PropertyObject* getChildObject(const ::mg::PropertyObject* obj) {
    using C = MemberClass<MemberPtr>;
    using T = MemberType<MemberPtr>;

    static_assert(ChildObjectTraits<T>::supported, "Member must expose a PropertyObject child.");

    return ChildObjectTraits<T>::getConst(static_cast<const C*>(obj)->*MemberPtr);
}

#if MG_BUILD_EDITOR
inline bool applyCollectionItemUpdates(::mg::PropertyObject& object, json::Reader updates) {
    if (!updates.isArray() || updates.size() == 0) return false;

    struct ResolvedUpdate {
        ::mg::PropertyObject* owner = nullptr;
        const ::mg::Property* property = nullptr;
        json::Reader value;
        std::string path;
    };
    std::vector<ResolvedUpdate> resolved;
    resolved.reserve(updates.size());

    const auto overlaps = [](std::string_view left, std::string_view right) {
        return left == right ||
               (left.size() > right.size() && left.starts_with(right) && left[right.size()] == '.') ||
               (right.size() > left.size() && right.starts_with(left) && right[left.size()] == '.');
    };
    for (std::size_t index = 0; index < updates.size(); ++index) {
        const json::Reader update = updates.element(index);
        std::string_view path;
        const json::Reader value = update.member("value");
        ::mg::PropertyObject* owner = nullptr;
        const ::mg::Property* property = nullptr;
        if (!update.isObject() || !update.member("path").read(path) || path.empty() || !value.valid() ||
            !object.resolvePath(std::string(path), owner, property) || !owner || !property ||
            !property->meta.validate || !property->meta.validate(owner, value)) return false;
        for (const ResolvedUpdate& previous : resolved)
            if (overlaps(path, previous.path)) return false;
        resolved.push_back({owner, property, value, std::string(path)});
    }

    for (const ResolvedUpdate& update : resolved)
        if (!update.owner->setProperty(update.property->key, update.value)) return false;
    return true;
}

template <typename Item>
::mg::PropertyObject* collectionItemObject(Item& item) {
    if constexpr (requires { { item.get() } -> std::convertible_to<::mg::PropertyObject*>; }) return item.get();
    else return &item;
}

template <auto MemberPtr>
    requires PropertyMember<MemberPtr> && InspectorCollectionTraits<MemberType<MemberPtr>>::supported
bool writeCollectionItems(const ::mg::PropertyObject* obj, json::Writer& writer) {
    using C = MemberClass<MemberPtr>;
    const C* self = static_cast<const C*>(obj);

    return writer.writeArray([&](json::ArrayWriter& items) {
        for (const auto& item : self->*MemberPtr) {
            if (!items.writeObject([&](json::ObjectWriter& entry) {
                const ::mg::PropertyObject* object = nullptr;
                if constexpr (requires { { item.get() } -> std::convertible_to<const ::mg::PropertyObject*>; }) object = item.get();
                else object = &item;
                if (!object) return entry.writeValue("type", [](json::Writer& value) { return value.null(); });
                if constexpr (requires { item->typeId(); })
                    if (!entry.write("type", object->typeId() ? object->typeId() : "")) return false;
                return entry.writeValue("inspector", [&](json::Writer& inspector) {
                    return object->writeInspectorMeta(inspector);
                });
            })) return false;
        }
        return true;
    });
}

template <auto MemberPtr>
    requires PropertyMember<MemberPtr> && InspectorCollectionTraits<MemberType<MemberPtr>>::supported &&
             requires (const MemberType<MemberPtr>& collection, json::Writer& writer) {
                 { InspectorCollectionTraits<MemberType<MemberPtr>>::writeDefault(collection, writer) } -> std::same_as<bool>;
             }
bool writeCollectionDefault(const ::mg::PropertyObject* obj, json::Writer& writer) {
    using C = MemberClass<MemberPtr>;
    return InspectorCollectionTraits<MemberType<MemberPtr>>::writeDefault(
        static_cast<const C*>(obj)->*MemberPtr, writer);
}

template <typename T>
bool cloneCollection(const T& source, T& destination) {
    json::Document document = json::array();
    json::Writer writer = document.writer();
    return encodeAny(writer, source) && decodeAny(document.root(), destination);
}

template <auto MemberPtr>
    requires PropertyMember<MemberPtr> && InspectorCollectionTraits<MemberType<MemberPtr>>::supported
bool mutateCollection(::mg::PropertyObject* obj, json::Reader operation) {
    using C = MemberClass<MemberPtr>;
    using T = MemberType<MemberPtr>;
    using Traits = InspectorCollectionTraits<T>;

    std::string_view action;
    if (!operation.isObject() || !operation.member("action").read(action)) return false;
    C* self = static_cast<C*>(obj);
    T next;
    if (!cloneCollection(self->*MemberPtr, next)) return false;

    if (action == "append") {
        typename T::value_type item;
        if (!decodeAny(operation.member("value"), item)) return false;
        next.push_back(std::move(item));
    } else {
        std::uint64_t rawIndex = 0;
        if (!operation.member("index").read(rawIndex) || rawIndex >= next.size()) return false;
        const std::size_t index = static_cast<std::size_t>(rawIndex);
        if (action == "remove") {
            next.erase(next.begin() + static_cast<std::ptrdiff_t>(index));
        } else if (action == "update") {
            ::mg::PropertyObject* item = collectionItemObject(next[index]);
            if (!item || !applyCollectionItemUpdates(*item, operation.member("updates"))) return false;
        } else {
            return false;
        }
    }

    if (!Traits::normalize(next)) return false;
    self->*MemberPtr = std::move(next);
    return true;
}

template <auto MemberPtr>
    requires PropertyMember<MemberPtr> && InspectorCollectionTraits<MemberType<MemberPtr>>::supported
const ::mg::CollectionMetadata& collectionMetadata() {
    static const ::mg::CollectionMetadata metadata = [] {
        ::mg::CollectionMetadata result{};
        result.getItems = &writeCollectionItems<MemberPtr>;
        result.mutate = &mutateCollection<MemberPtr>;
        if constexpr (requires(const MemberType<MemberPtr>& collection, json::Writer& writer) {
            { InspectorCollectionTraits<MemberType<MemberPtr>>::writeDefault(collection, writer) } -> std::same_as<bool>;
        }) result.getDefault = &writeCollectionDefault<MemberPtr>;
        return result;
    }();
    return metadata;
}
#endif

} // namespace detail

template <auto MemberPtr>
::mg::Property makeProperty(const char* key)
    requires detail::PropertyMember<MemberPtr> {
    using T = detail::MemberType<MemberPtr>;
    ::mg::Property::ChildGetter child = nullptr;
    if constexpr (detail::ChildObjectTraits<T>::supported) child = &detail::getChildObject<MemberPtr>;

#if MG_BUILD_EDITOR
    ::mg::PropertyMetadata meta{};
    meta.nullable = PropertyNullableTraits<T>::value;
    meta.validate = &detail::validateMember<MemberPtr>;
    if constexpr (::mg::EnumDescribed<::mg::EnumTraitsTypeT<T>>) meta.getOptions = &::mg::enumOptionsMeta<::mg::EnumTraitsTypeT<T>>;
    if constexpr (::mg::MgPolymorphicRegistryTraits<T>::supported) meta.getTypes = &::mg::MgPolymorphicRegistryTraits<T>::getTypesMeta;
    if constexpr (requires(::mg::json::Writer& writer) { { ::mg::MgPolymorphicRegistryTraits<T>::getDefaultMeta(writer) } -> std::same_as<bool>; }) {
        meta.getDefault = &::mg::MgPolymorphicRegistryTraits<T>::getDefaultMeta;
    }
    if constexpr (detail::InspectorCollectionTraits<T>::supported) {
        meta.collection = &detail::collectionMetadata<MemberPtr>();
        if constexpr (requires(json::Writer& writer) { { detail::InspectorCollectionTraits<T>::getTypesMeta(writer) } -> std::same_as<bool>; })
            meta.getTypes = &detail::InspectorCollectionTraits<T>::getTypesMeta;
    }
    return {key, &detail::setMember<MemberPtr>, &detail::getMember<MemberPtr>, child, meta};
#else
    return {key, &detail::setMember<MemberPtr>, &detail::getMember<MemberPtr>, child};
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

#define MG_PROP(member, key) ::mg::props::makeProperty<&Self::member>(key),

#define MG_PROP_CUSTOM(key, set_fn, get_fn) ::mg::props::makeCustomProperty(key, set_fn, get_fn),

#define MG_PROPS_END() \
        }; \
        const auto parentGetter = ::mg::props::detail::parentPropertyListGetter<Self>(); \
        return { props, sizeof(props) / sizeof(props[0]), parentGetter }; \
    }
