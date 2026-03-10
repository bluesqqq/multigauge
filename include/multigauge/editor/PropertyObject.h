#pragma once

#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>
#include <vector>

#include <rapidjson/document.h>
#include <multigauge/editor/Codec.h>

#include <multigauge/editor/Property.h>

#define TYPE_KEY "type"

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
        virtual const char* typeName() const { return "PropertyObject"; }

        struct PropertyList {
            using ParentGetter = PropertyList (*)(const PropertyObject*);

            const Property* props;
            std::size_t count;
            ParentGetter parent;
        };

        virtual ~PropertyObject() = default;

        virtual const char* typeId() const { return nullptr; }

        //----------[ PROPERTIES ]----------//

        /// @brief Returns the list of all properties. Override this to set properties, see macros below
        virtual PropertyList propertyList() const { return {nullptr, 0, nullptr}; }

        /// @brief Finds a single property by key
        const Property* findProperty(const char* key) const;

        /// @brief Loads a single property from JSON
        bool loadProperty(const char* key, const rapidjson::Value& v);

        /// @brief Saves a single property as JSON
        bool saveProperty(const char* key, rapidjson::Value& out, rapidjson::Document::AllocatorType& a) const;

        /// @brief Loads all properties from JSON
        void loadProperties(rapidjson::Value::ConstObject json);

        /// @brief Saves all properties to JSON
        void saveProperties(rapidjson::Value& out, rapidjson::Document::AllocatorType& a) const;

        /// @brief Returns a list of all properties that are 
        std::vector<const Property*> getPropertyObjectProperties() const;

    protected:
        //----------[ Member-pointer traits ]----------//
        template <typename M> struct MemberPtrTraits;

        template <typename C, typename T>
        struct MemberPtrTraits<T C::*> {
            using Class = C;
            using Type  = T;
        };

        template <auto MemberPtr>
        using MemberClass = typename MemberPtrTraits<decltype(MemberPtr)>::Class;

        template <auto MemberPtr>
        using MemberType = typename MemberPtrTraits<decltype(MemberPtr)>::Type;

        //----------[ GET + SET ]----------//

        template <auto MemberPtr, auto CallbackPtr>
        static bool setMember(PropertyObject* obj, const rapidjson::Value& v) {
            using C = MemberClass<MemberPtr>;
            using T = MemberType<MemberPtr>;
            C* self = static_cast<C*>(obj);

            T decoded{};
            if (!decodeAny<T>(v, decoded)) return false;
            self->*MemberPtr = std::move(decoded);

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

        template <typename Base>
        static PropertyList getParentPropertyList(const PropertyObject* obj) {
            if (!obj) return {nullptr, 0, nullptr};
            return static_cast<const Base*>(obj)->Base::propertyList();
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

#define MG_EDITOR_NAME(name_literal) \
    public: const char* typeName() const override { return name_literal; }

#define MG_TYPE_ID(str_literal) \
    public: const char* typeId() const override { return (str_literal); }

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
    ::Property{ \
        key, \
        display_name ? display_name : key, \
        description ? description : "No description.", \
        &PropertyObject::setMember<&Self::member, nullptr>, \
        &PropertyObject::getMember<&Self::member> \
    },

#define MG_PROP_CALLBACK(member, key, display_name, description, callback) \
    ::Property{ \
        key, \
        display_name ? display_name : key, \
        description ? description : "No description.", \
        &PropertyObject::setMember<&Self::member, callback>, \
        &PropertyObject::getMember<&Self::member> \
    },

#define MG_PROP_CUSTOM(key, display_name, description, set, get) \
    ::Property{ \
        key, \
        display_name ? display_name : key, \
        description ? description : "No description.", \
        set, \
        get \
    },

#define MG_PROPS_END() \
        }; \
        const auto parentGetter = ::MgPropsParentGetter<Self>::value; \
        return { props, sizeof(props) / sizeof(props[0]), parentGetter }; \
    }
