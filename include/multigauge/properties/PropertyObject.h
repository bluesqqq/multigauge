#pragma once

#include <cstddef>
#include <concepts>
#include <string>
#include <vector>

#include <multigauge/json/Json.h>
#include <multigauge/properties/Property.h>

#define TYPE_KEY "type"

namespace mg {

class PropertyObject {
    public:
        /// Default destructor.
        virtual ~PropertyObject() = default;

        /// Returns the human-readable type name for this object.
        virtual const char* typeName() const { return "PropertyObject"; }

        /// Returns the serialized type identifier for this object.
        virtual const char* typeId() const { return nullptr; }

        struct PropertyList {
            using ParentGetter = PropertyList (*)(const PropertyObject*);

            const Property* props;
            std::size_t count;
            ParentGetter parent;

            /// Returns the next property list in the inheritance chain.
            static PropertyList next(const PropertyObject* self, PropertyList current) {
                if (!current.parent) return {};
                return current.parent(self);
            }

            /// Returns whether this property list contains data or a parent chain.
            bool valid() const { return props != nullptr || parent != nullptr; }

            /// Iterates this property list and every parent list in order.
            template <typename Fn>
                requires std::invocable<Fn&, const Property&>
            void forEach(const PropertyObject* self, Fn&& fn) const {
                for (PropertyList pl = *this; pl.valid(); pl = next(self, pl)) {
                    if (!pl.props || pl.count == 0) continue;
                    for (std::size_t i = 0; i < pl.count; ++i) {
                        fn(pl.props[i]);
                    }
                }
            }
        };

        /// Returns the list of properties exposed by this object.
        virtual PropertyList propertyList() const { return {nullptr, 0, nullptr}; }

        /// Finds a property by key.
        const Property* findProperty(std::string_view key) const;

        /// Sets a single property from a JSON value.
        /// @return `true` if the property exists and accepts the value, otherwise `false`.
        bool setProperty(std::string_view key, json::Reader value);

        /// Serializes a single property to JSON.
        /// @return `true` if the property exists and is successfully serialized, otherwise `false`.
        bool getProperty(std::string_view key, json::Writer& writer) const;

        /// Loads all matching properties from a JSON object.
        bool loadProperties(json::Reader object);

        /// Serializes all exposed properties into a JSON object.
        bool saveProperties(json::Writer& writer) const;

        /// Serializes the type identifier and all exposed properties into an
        /// already-open JSON object. This composes property objects with
        /// container-owned fields such as a gauge face's child hierarchy.
        bool savePropertyMembers(json::ObjectWriter& object) const;

        /// Builds editor metadata for every visible property on this object.
        /// @see docs/schemas/rules.schema.json for visibility and interactability rules.
        bool writePropertiesMeta(json::Writer& writer) const;

        /// Builds editor metadata for a single property.
        /// @param prop The property to describe.
        bool writePropertyMeta(json::Writer& writer, const Property& prop) const;

        /// Resolves a dotted property path to its owning object and final property.
        /// @param path Dotted path such as `"layout.margin.left"`.
        /// @param owner Output pointer receiving the object that owns the final property.
        /// @param prop Output pointer receiving the resolved property.
        /// @return `true` if the full path resolves successfully, otherwise `false`.
        bool resolvePath(const std::string& path, PropertyObject*& owner, const Property*& prop);

        /// Const overload of `resolvePath`.
        /// @param path Dotted path such as `"layout.margin.left"`.
        /// @param owner Output pointer receiving the object that owns the final property.
        /// @param prop Output pointer receiving the resolved property.
        /// @return `true` if the full path resolves successfully, otherwise `false`.
        bool resolvePath(const std::string& path, const PropertyObject*& owner, const Property*& prop) const;

    protected:
        /// Splits a dotted property path into path segments.
        static std::vector<std::string> splitPath(const std::string& path);
};

} // namespace mg

//----------[ MACROS ]----------//

#define MG_EDITOR_NAME(name_literal) \
    public: static constexpr const char* staticTypeName() { return name_literal; } \
    public: const char* typeName() const override { return staticTypeName(); }

#define MG_TYPE_ID(str_literal) \
    public: static constexpr const char* staticTypeId() { return (str_literal); } \
    public: const char* typeId() const override { return staticTypeId(); }

#include <multigauge/properties/PropertyBuilder.h>
