#pragma once

#include <multigauge/Config.h>

#if MG_BUILD_EDITOR
#include <initializer_list>
#include <string>
#include <unordered_set>

#include <multigauge/json/Json.h>
#include <multigauge/properties/meta/PropertyMetadata.h>

namespace mg {

class PropertyObject; // Forward declaration

namespace inspector {

struct Binding {
    const char* name;
    const char* path;
};

struct Option {
    const char* key;
    const char* value;
};

/// @brief Builds an editor-only inspector presentation tree for one property object.
class Builder {
public:
    Builder(
        const PropertyObject& object,
        json::ArrayWriter& nodes,
        std::unordered_set<std::string>& bindings
    ) : object_(object),
        nodes_(nodes),
        bindings_(bindings) {}

    template <typename Fn>
    bool section(const char* title, Fn&& fn) {
        return nodes_.writeObject([&](json::ObjectWriter& object) {
            return object.write("type", "section") && object.write("title", title) &&
                   object.writeArray("children", [&](json::ArrayWriter& children) {
                       Builder child{object_, children, bindings_};
                       return fn(child);
                   });
        });
    }

    template <typename Fn>
    bool row(Fn&& fn) {
        return nodes_.writeObject([&](json::ObjectWriter& object) {
            return object.write("type", "row") && object.writeArray("children", [&](json::ArrayWriter& children) {
                Builder child{object_, children, bindings_};
                return fn(child);
            });
        });
    }

    bool property(
        const char* path,
        PropertyMetadata::RuleListGetter visibleWhen = nullptr
    );
    
    bool control(
        const char* widget, std::initializer_list<Binding> bindings,
        PropertyMetadata::RuleListGetter visibleWhen = nullptr,
        std::initializer_list<Option> options = {}
    );

private:
    bool claim(const char* path);

    const PropertyObject& object_;
    json::ArrayWriter& nodes_;
    std::unordered_set<std::string>& bindings_;
};

} // namespace inspector

} // namespace mg
#endif

#if MG_BUILD_EDITOR
#define MG_INSPECTOR_BEGIN() \
protected: \
    bool hasInspectorLayout() const override { return true; } \
    bool writeInspectorLayout(::mg::json::Writer& writer) const override { \
        return writer.writeArray([&](::mg::json::ArrayWriter& inspectorNodes) { \
            std::unordered_set<std::string> inspectorBindings; \
            ::mg::inspector::Builder inspector{*this, inspectorNodes, inspectorBindings};

#define MG_INSPECTOR_END() \
            return true; \
        }); \
    }

#define MG_BIND(name, path) ::mg::inspector::Binding{name, path}
#define MG_OPTION(key, value) ::mg::inspector::Option{key, value}
#else
#define MG_INSPECTOR_BEGIN()
#define MG_INSPECTOR_END()
#define MG_BIND(name, path)
#define MG_OPTION(key, value)
#endif
