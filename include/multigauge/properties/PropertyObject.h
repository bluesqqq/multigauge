#pragma once

#include <cstddef>
#include <cstring>
#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <rapidjson/document.h>
#include <multigauge/properties/Codec.h>
#include <multigauge/properties/PolymorphicRegistry.h>

#include <multigauge/properties/Property.h>

#define TYPE_KEY "type"

struct rgba;
class PropertyObject;

template <typename T, typename = void>
struct MgPropWidgetTraits;
template <typename T>
struct MgPropNullableTraits {
    static constexpr bool value = false;
};


//----------[ ENCODE + DECODE ]----------//

template <typename T>
inline bool decodeAny(const rapidjson::Value& v, T& out) {
    // Codec<T>
    if constexpr (HasCodecV<T>) {
        if (Codec<T>::decode(v, out)) return true;
    }

    // PropertyObject-derived type
    if constexpr (std::is_base_of_v<PropertyObject, T>) {
        if (!v.IsObject()) return false;
        out.loadProperties(v.GetObject());
        return true;
    }

    static_assert(HasCodecV<T> || std::is_base_of_v<PropertyObject, T>, "Type must have Codec<T> or derive from PropertyObject.");
    return false;
}

template <typename T>
inline bool encodeAny(rapidjson::Value& out, rapidjson::Document::AllocatorType& a, const T& v) {
    // Codec<T>
    if constexpr (HasCodecV<T>) {
        if (Codec<T>::encode(out, a, v)) return true;
    }

    // PropertyObject-derived type
    if constexpr (std::is_base_of_v<PropertyObject, T>) {
        v.saveProperties(out, a);
        return true;
    }

    static_assert(HasCodecV<T> || std::is_base_of_v<PropertyObject, T>, "Type must have Codec<T> with encode/decode or derive from PropertyObject.");
    return false;
}

class PropertyObject {
public:
    virtual ~PropertyObject() = default;

    virtual const char* typeName() const { return "PropertyObject"; }
    virtual const char* typeId() const { return nullptr; }

    struct PropertyList {
        using ParentGetter = PropertyList (*)(const PropertyObject*);

        const Property* props;
        std::size_t count;
        ParentGetter parent;

        static PropertyList next(const PropertyObject* self, PropertyList current) {
            if (!current.parent) return {};
            return current.parent(self);
        }

        bool valid() const { return props != nullptr || parent != nullptr; }

        template <typename Fn>
        void forEach(const PropertyObject* self, Fn&& fn) const {
            for (PropertyList pl = *this; pl.valid(); pl = next(self, pl)) {
                if (!pl.props || pl.count == 0) continue;
                for (std::size_t i = 0; i < pl.count; ++i) {
                    fn(pl.props[i]);
                }
            }
        }
    };

    virtual PropertyList propertyList() const { return {nullptr, 0, nullptr}; }

    const Property* findProperty(const char* key) const;

    bool loadProperty(const char* key, const rapidjson::Value& v);
    bool saveProperty(const char* key, rapidjson::Value& out, rapidjson::Document::AllocatorType& a) const;

    void loadProperties(rapidjson::Value::ConstObject json);
    void saveProperties(rapidjson::Value& out, rapidjson::Document::AllocatorType& a) const;

    rapidjson::Value getPropertiesMeta(rapidjson::Document::AllocatorType& a) const;
    rapidjson::Value getPropertyMeta(const Property& prop, rapidjson::Document::AllocatorType& a) const;

    bool resolvePath(const std::string& path, PropertyObject*& owner, const Property*& prop);
    bool resolvePath(const std::string& path, const PropertyObject*& owner, const Property*& prop) const;

protected:
    static std::vector<std::string> splitPath(const std::string& path);

    template <typename M> struct MemberPtrTraits;

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
    static bool setMember(PropertyObject* obj, const rapidjson::Value& v) {
        using C = MemberClass<MemberPtr>;
        using T = MemberType<MemberPtr>;
        C* self = static_cast<C*>(obj);

        if constexpr (std::is_base_of_v<PropertyObject, T>) {
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
    static bool getMember(const PropertyObject* obj, rapidjson::Value& out, rapidjson::Document::AllocatorType& a) {
        using C = MemberClass<MemberPtr>;
        const C* self = static_cast<const C*>(obj);
        return encodeAny(out, a, self->*MemberPtr);
    }

        template <typename T, typename = void>
    struct ChildObjectTraits {
        static constexpr bool supported = false;

        static const PropertyObject* getConst(const T&) { return nullptr; }
        static PropertyObject* getMutable(T&) { return nullptr; }
    };

    template <typename T>
    struct ChildObjectTraits<T, std::enable_if_t<std::is_base_of_v<PropertyObject, T>>> {
        static constexpr bool supported = true;

        static const PropertyObject* getConst(const T& value) { return &value; }
        static PropertyObject* getMutable(T& value) { return &value; }
    };

    template <typename T>
    struct ChildObjectTraits<std::unique_ptr<T>, std::enable_if_t<std::is_base_of_v<PropertyObject, T>>> {
        static constexpr bool supported = true;

        static const PropertyObject* getConst(const std::unique_ptr<T>& value) { return value.get(); }
        static PropertyObject* getMutable(std::unique_ptr<T>& value) { return value.get(); }
    };

    template <typename T>
    struct ChildObjectTraits<std::optional<T>, std::enable_if_t<std::is_base_of_v<PropertyObject, T>>> {
        static constexpr bool supported = true;

        static const PropertyObject* getConst(const std::optional<T>& value) {
            return value ? &(*value) : nullptr;
        }

        static PropertyObject* getMutable(std::optional<T>& value) {
            return value ? &(*value) : nullptr;
        }
    };

    template <auto MemberPtr>
    static const PropertyObject* getChildObject(const PropertyObject* obj) {
        using C = MemberClass<MemberPtr>;
        using T = MemberType<MemberPtr>;
        static_assert(ChildObjectTraits<T>::supported, "Member must expose a PropertyObject child.");
        const C* self = static_cast<const C*>(obj);
        return ChildObjectTraits<T>::getConst(self->*MemberPtr);
    }

    template <auto MemberPtr>
    static PropertyObject* getChildObjectMutable(PropertyObject* obj) {
        using C = MemberClass<MemberPtr>;
        using T = MemberType<MemberPtr>;
        static_assert(ChildObjectTraits<T>::supported, "Member must expose a PropertyObject child.");
        C* self = static_cast<C*>(obj);
        return ChildObjectTraits<T>::getMutable(self->*MemberPtr);
    }

    template <typename Base>
    static PropertyList getParentPropertyList(const PropertyObject* obj) {
        if (!obj) return {nullptr, 0, nullptr};
        return static_cast<const Base*>(obj)->Base::propertyList();
    }

    template <typename T>
    static rapidjson::Value getEnumOptions(rapidjson::Document::AllocatorType& a) {
        return enumOptionsMeta<EnumTraitsTypeT<T>>(a);
    }

    template <typename T>
    static rapidjson::Value getPolymorphicTypes(rapidjson::Document::AllocatorType& a) {
        return MgPolymorphicRegistryTraits<T>::getTypesMeta(a);
    }


    template <auto MemberPtr, auto CallbackPtr = nullptr>
    static Property makeProperty(const char* key, const char* name, const char* description, PropertyMetadata::RuleListGetter visibleWhen = nullptr, PropertyMetadata::RuleListGetter interactableWhen = nullptr, bool inspectorVisible = true) {
        using T = MemberType<MemberPtr>;

        Property p{};
        p.key = key;
        p.set = &PropertyObject::setMember<MemberPtr, CallbackPtr>;
        p.get = &PropertyObject::getMember<MemberPtr>;

        PropertyMetadata meta{};
        meta.name = name ? name : key;
        meta.description = description ? description : "No description.";
        meta.widget = MgPropWidgetTraits<T>::value;
        meta.nullable = MgPropNullableTraits<T>::value;
        meta.inspectorVisible = inspectorVisible;
        meta.getVisibleWhen = visibleWhen;
        meta.getInteractableWhen = interactableWhen;
        if constexpr (HasEnumTraitsV<EnumTraitsTypeT<T>>) meta.getOptions = &PropertyObject::getEnumOptions<T>;
        if constexpr (MgPolymorphicRegistryTraits<T>::supported) meta.getTypes = &PropertyObject::getPolymorphicTypes<T>;

        if constexpr (ChildObjectTraits<T>::supported) {
            p.getChild = &PropertyObject::getChildObject<MemberPtr>;
            p.getChildMutable = &PropertyObject::getChildObjectMutable<MemberPtr>;
        }

        return {p.key, p.set, p.get, p.getChild, p.getChildMutable, meta};
    }

    static Property makeCustomProperty(const char* key, const char* name, const char* description, PropertyMetadata::RuleListGetter visibleWhen, PropertyMetadata::RuleListGetter interactableWhen, bool inspectorVisible, Property::Setter set, Property::Getter get) {
        PropertyMetadata meta{};
        meta.name = name ? name : key;
        meta.description = description ? description : "No description.";
        meta.widget = "json";
        meta.inspectorVisible = inspectorVisible;
        meta.getVisibleWhen = visibleWhen;
        meta.getInteractableWhen = interactableWhen;

        return {key, set, get, nullptr, nullptr, meta};
    }
};

//----------[ MACROS ]----------//

template <typename T, typename = void>
struct MgPropsParentGetter {
    static constexpr PropertyObject::PropertyList::ParentGetter value = nullptr;
};

template <typename T>
struct MgPropsParentGetter<T, std::void_t<decltype(&T::__mg_parent_property_list)>> {
    static constexpr PropertyObject::PropertyList::ParentGetter value = &T::__mg_parent_property_list;
};

template <typename T, typename>
struct MgPropWidgetTraits { static constexpr const char* value = "json"; };

template <typename T>
struct MgPropWidgetTraits<T, std::enable_if_t<HasEnumTraitsV<EnumTraitsTypeT<T>>>> {
    static constexpr const char* value = "select";
};

#define MG_EDITOR_WIDGET(type, widget_literal) \
template <> \
struct MgPropWidgetTraits<type> { static constexpr const char* value = widget_literal; };

MG_EDITOR_WIDGET(bool, "boolean")
MG_EDITOR_WIDGET(rgba, "color-selector")

template <typename T>
struct MgPropWidgetTraits<T, std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>> {
    static constexpr const char* value = "number";
};

MG_EDITOR_WIDGET(std::string, "string")

template <typename T>
struct MgPropWidgetTraits<std::optional<T>> {
    static constexpr const char* value = MgPropWidgetTraits<T>::value;
};

template <typename T>
struct MgPropWidgetTraits<std::vector<T>> {
    static constexpr const char* value = "array";
};

template <typename T>
struct MgPropNullableTraits<std::optional<T>> {
    static constexpr bool value = true;
};

template <typename T>
struct MgPropWidgetTraits<T, std::enable_if_t<std::is_base_of_v<PropertyObject, T>>> {
    static constexpr const char* value = "group";
};

template <typename T>
struct MgPropWidgetTraits<std::unique_ptr<T>> {
    static constexpr const char* value = MgPropWidgetTraits<T>::value;
};

#define MG_EDITOR_NAME(name_literal) \
    public: static constexpr const char* staticTypeName() { return name_literal; } \
    public: const char* typeName() const override { return staticTypeName(); }

#define MG_TYPE_ID(str_literal) \
    public: static constexpr const char* staticTypeId() { return (str_literal); } \
    public: const char* typeId() const override { return staticTypeId(); }

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
    ::PropertyObject::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        true \
    ),

#define MG_PROP_CALLBACK(member, key, display_name, description, callback) \
    ::PropertyObject::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        true \
    ),

#define MG_PROP_UI(member, key, display_name, description, visible_when, interactable_when) \
    ::PropertyObject::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        true \
    ),

#define MG_PROP_CALLBACK_UI(member, key, display_name, description, callback, visible_when, interactable_when) \
    ::PropertyObject::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        true \
    ),

#define MG_PROP_HIDDEN(member, key, display_name, description) \
    ::PropertyObject::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        false \
    ),

#define MG_PROP_CALLBACK_HIDDEN(member, key, display_name, description, callback) \
    ::PropertyObject::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        nullptr, \
        nullptr, \
        false \
    ),

#define MG_PROP_UI_HIDDEN(member, key, display_name, description, visible_when, interactable_when) \
    ::PropertyObject::makeProperty<&Self::member, nullptr>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        false \
    ),

#define MG_PROP_CALLBACK_UI_HIDDEN(member, key, display_name, description, callback, visible_when, interactable_when) \
    ::PropertyObject::makeProperty<&Self::member, callback>( \
        key, \
        display_name, \
        description, \
        visible_when, \
        interactable_when, \
        false \
    ),

#define MG_PROP_CUSTOM(key, display_name, description, set_fn, get_fn) \
    ::PropertyObject::makeCustomProperty( \
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
    ::PropertyObject::makeCustomProperty( \
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
