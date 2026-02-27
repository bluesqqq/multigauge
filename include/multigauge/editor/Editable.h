#pragma once

#include <cstddef>
#include <cstring>
#include <utility>
#include <vector>
#include <type_traits>

#include <rapidjson/document.h>
#include <multigauge/editor/Codec.h>


class Editable {
    public:
        struct Property {
            const char* name;
            bool (*setFn)(Editable* obj, const rapidjson::Value& v);
        };

        struct PropertyList {
            const Property* props;
            std::size_t count;
        };

        virtual ~Editable() = default;

        // These two are what your macros override.
        virtual PropertyList propertyList() const { return {nullptr, 0}; }

        const Property* findProperty(const char* name) const {
            if (!name) return nullptr;

            PropertyList pl = propertyList();
            if (!pl.props || pl.count == 0) return nullptr;

            for (std::size_t i = 0; i < pl.count; ++i) {
                const char* propName = pl.props[i].name;
                if (propName && std::strcmp(propName, name) == 0)
                    return &pl.props[i];
            }
            return nullptr;
        }

        bool set(const char* name, const rapidjson::Value& v) {
            const Property* p = findProperty(name);
            if (!p || !p->setFn) return false;
            return p->setFn(this, v);
        }

        void loadProperties(rapidjson::Value::ConstObject json) {
            for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it) {
                const char* key = it->name.GetString();
                set(key, it->value);
            }
        }

    protected:
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

        // ---- HasCodec trait ----
        template <typename T, typename = void>
        struct HasCodec : std::false_type {};

        template <typename T>
        struct HasCodec<T, std::void_t<
            decltype(Codec<T>::decode(std::declval<const rapidjson::Value&>(), std::declval<T&>()))
        >> : std::true_type {};

        // -------- Member-pointer traits --------
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

        template <typename E>
        static bool decodeElement(const rapidjson::Value& v, E& out) {
            if constexpr (HasCodec<E>::value) {
                return Codec<E>::decode(v, out);
            } else if constexpr (std::is_base_of_v<Editable, E>) {
                if (!v.IsObject()) return false;
                out.loadProperties(v.GetObject());
                return true;
            } else {
                static_assert(
                    HasCodec<E>::value || std::is_base_of_v<Editable, E>,
                    "std::vector element type must have Codec<T> or derive from Editable."
                );
                return false;
            }
        }

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
                    if (!decodeElement<E>(*it, elem)) return false;
                    tmp.push_back(std::move(elem));
                }

                self->*MemberPtr = std::move(tmp);
                return true;
            } else {
                T decoded{};
                if (!decodeElement<T>(v, decoded)) return false;
                self->*MemberPtr = std::move(decoded);
                return true;
            }
        }

        template <auto MemberPtr>
        static constexpr Property makeProperty(const char* name) {
            return Property{ name, &Editable::setMember<MemberPtr> };
        }
};

// -------- Macros --------
//
// Usage:
//   class X : public Editable {
//     int a;
//     MG_EDITABLE_BEGIN()
//       MG_PROP(a)
//     MG_EDITABLE_END()
//   };
//
#define MG_EDITABLE_BEGIN()                                                  \
public:                                                                      \
    ::Editable::PropertyList propertyList() const override {                 \
        using Self = std::remove_cv_t<std::remove_reference_t<decltype(*this)>>; \
        static const ::Editable::Property props[] = {

#define MG_PROP(member)                                                      \
            ::Editable::makeProperty<&Self::member>(#member),

#define MG_EDITABLE_END()                                                    \
        };                                                                   \
        return { props, sizeof(props) / sizeof(props[0]) };                  \
    }