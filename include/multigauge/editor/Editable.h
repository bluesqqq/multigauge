#pragma once

#include <cstddef>
#include <cstring>
#include <type_traits>
#include <utility>
#include <vector>

#include <rapidjson/document.h>
#include <multigauge/editor/Codec.h>

#include <multigauge/editor/Property.h>

template <typename T>
inline bool decodeAny(const rapidjson::Value& v, T& out) {
    if constexpr (HasCodecV<T>) {
        if (Codec<T>::decode(v, out)) return true;
    }

    if constexpr (std::is_base_of_v<Editable, T>) {
        if (!v.IsObject()) return false;
        out.loadProperties(v.GetObject());
        return true;
    }

    static_assert(HasCodecV<T> || std::is_base_of_v<Editable, T>, "Type must have Codec<T> or derive from Editable.");
    return false;
}

template <typename T>
inline bool encodeAny(rapidjson::Value& out, rapidjson::Document::AllocatorType& a, const T& v) {
    if constexpr (HasCodecV<T>) {
        if (Codec<T>::encode(out, a, v)) return true;
    }

    if constexpr (std::is_base_of_v<Editable, T>) {
        v.saveProperties(out, a);
        return true;
    }

    static_assert(HasCodecV<T> || std::is_base_of_v<Editable, T>, "Type must have Codec<T> with encode/decode or derive from Editable.");
    return false;
}

class Editable {
    public:
        virtual const char* editorTypeName() const { return "Editable"; }

        struct PropertyList {
            const Property* props;
            std::size_t count;
        };

        virtual ~Editable() = default;

        //----------[ PROPERTIES ]----------//

        /// @brief Returns the list of all properties. Override this to set properties, see macros below
        virtual PropertyList propertyList() const { return {nullptr, 0}; }

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
        std::vector<const Property*> getEditableProperties() const;

    protected:
        //----------[ std::vector traits ]----------//
        template <typename T>
        struct IsStdVector : std::false_type {};

        template <typename U, typename Alloc>
        struct IsStdVector<std::vector<U, Alloc>> : std::true_type {};

        template <typename T>
        inline static constexpr bool IsStdVectorV = IsStdVector<T>::value;

        template <typename T>
        struct VectorElem;

        template <typename U, typename Alloc>
        struct VectorElem<std::vector<U, Alloc>> { using Type = U; };

        template <typename T>
        using VectorElemT = typename VectorElem<T>::Type;

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

        template <auto MemberPtr>
        static bool setMember(Editable* obj, const rapidjson::Value& v) {
            using C = MemberClass<MemberPtr>;
            using T = MemberType<MemberPtr>;

            C* self = static_cast<C*>(obj);

            if constexpr (IsStdVectorV<T>) {
                using E = VectorElemT<T>;

                if (!v.IsArray()) return false;

                T tmp;
                tmp.reserve(v.Size());

                for (auto it = v.Begin(); it != v.End(); ++it) {
                    E elem{};
                    if (!decodeAny<E>(*it, elem)) return false;
                    tmp.push_back(std::move(elem));
                }

                self->*MemberPtr = std::move(tmp);
                return true;
            } else {
                T decoded{};
                if (!decodeAny<T>(v, decoded)) return false;
                self->*MemberPtr = std::move(decoded);
                return true;
            }
        }

        template <auto MemberPtr, auto CallbackPtr>
        static bool setMemberThen(Editable* obj, const rapidjson::Value& v) {
            if (!setMember<MemberPtr>(obj, v)) return false;

            using CbClass = MemberClass<CallbackPtr>;
            auto* self = static_cast<CbClass*>(obj);
            (self->*CallbackPtr)();
            return true;
        }

        template <auto MemberPtr>
        static bool getMember(const Editable* obj, rapidjson::Value& out, rapidjson::Document::AllocatorType& a) {
            using C = MemberClass<MemberPtr>;
            using T = MemberType<MemberPtr>;

            const C* self = static_cast<const C*>(obj);

            if constexpr (IsStdVectorV<T>) {
                out.SetArray();
                const T& vec = self->*MemberPtr;
                out.Reserve(static_cast<rapidjson::SizeType>(vec.size()), a);

                for (const auto& e : vec) {
                    rapidjson::Value elem;
                    if (!encodeAny(elem, a, e)) return false;
                    out.PushBack(elem, a);
                }
                return true;
            } else {
                return encodeAny(out, a, self->*MemberPtr);
            }
        }

        template <auto MemberPtr>
        static constexpr Property makeProperty(const char* key, const char* name = nullptr, const char* desc = nullptr) {
            return Property{ key, name ? name : key, desc ? desc : "No description.", &Editable::setMember<MemberPtr>, &Editable::getMember<MemberPtr> };
        }

        template <auto MemberPtr, auto CallbackPtr>
        static constexpr Property makeProperty(const char* key, const char* name = nullptr, const char* desc = nullptr) {
            return Property{ key, name ? name : key, desc ? desc : "No description.", &Editable::setMemberThen<MemberPtr, CallbackPtr>, &Editable::getMember<MemberPtr> };
        }
};

// -------- Macros --------

// Usage:
//   class X : public Editable {
//     int a;
//     MG_EDITOR_BEGIN()
//       MG_EDITOR_PROP(a)
//     MG_EDITOR_END()
//   };
//
#define MG_EDITOR_NAME(name_literal) \
    public: const char* editorTypeName() const override { return name_literal; }

#define MG_EDITOR_BEGIN() \
public: \
    ::Editable::PropertyList propertyList() const override { \
        using Self = std::remove_cv_t<std::remove_reference_t<decltype(*this)>>; \
        static const ::Property props[] = {



#define MG_EDITOR_PROP(member, key, display_name, description) \
    ::Editable::makeProperty<&Self::member>(key, display_name, description),

#define MG_EDITOR_PROP_CALLBACK(member, key, callback, display_name, description) \
    ::Editable::makeProperty<&Self::member, callback>(key, display_name, description),



#define MG_EDITOR_END() \
        }; \
        return { props, sizeof(props) / sizeof(props[0]) }; \
    }