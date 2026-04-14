#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <rapidjson/document.h>
#include <multigauge/properties/Property.h>

#define TYPE_KEY "type"

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
};

//----------[ MACROS ]----------//

#define MG_EDITOR_NAME(name_literal) \
    public: static constexpr const char* staticTypeName() { return name_literal; } \
    public: const char* typeName() const override { return staticTypeName(); }

#define MG_TYPE_ID(str_literal) \
    public: static constexpr const char* staticTypeId() { return (str_literal); } \
    public: const char* typeId() const override { return staticTypeId(); }

#include <multigauge/properties/PropertyBuilder.h>
