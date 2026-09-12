#pragma once

#include <multigauge/Config.h>

#if MG_BUILD_EDITOR
#include <initializer_list>
#include <string>
#include <unordered_set>

#include <multigauge/json/Json.h>
#include <multigauge/properties/meta/Rules.h>
#include <multigauge/properties/meta/Widgets.h>

namespace mg {

class PropertyObject; // Forward declaration

namespace inspector {

struct Binding {
    const char* name;
    const char* path;
};

/// @brief A simple inspector visibility condition.
struct Rule {
    const char* path;
    const char* op;
    std::initializer_list<const char*> values;
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

    template <typename Fn>
    bool row(const char* label, Fn&& fn) {
        return nodes_.writeObject([&](json::ObjectWriter& object) {
            return object.write("type", "row") && object.write("label", label) &&
                   object.writeArray("children", [&](json::ArrayWriter& children) {
                       Builder child{object_, children, bindings_};
                       return fn(child);
                   });
        });
    }

    bool property(const char* path, const char* label, const char* widget);
    bool property(const char* path, const char* label, const char* widget, const Rule& visibleWhen);

    /// Includes a child property object's inspector sections at the current level.
    bool include(const char* path);

    bool control(const char* widget, std::initializer_list<Binding> bindings);
    bool control(
        const char* widget, std::initializer_list<Binding> bindings,
        const Rule& visibleWhen
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

#define MG_SECTION(title, body) \
    do { \
        if (!inspector.section(title, [&](auto& inspector) -> bool { \
                body \
                return true; \
            })) return false; \
    } while (false)

#define MG_ROW(body) \
    do { \
        if (!inspector.row([&](auto& inspector) -> bool { \
                body \
                return true; \
            })) return false; \
    } while (false)

#define MG_PROPERTY(path, label, widget) \
    do { \
        if (!inspector.property(path, label, widget)) return false; \
    } while (false)

#define MG_PROPERTY_IF(path, label, widget, visible_when) \
    do { \
        if (!inspector.property(path, label, widget, visible_when)) return false; \
    } while (false)

#define MG_LABELED_ROW(label, body) \
    do { \
        if (!inspector.row(label, [&](auto& inspector) -> bool { \
                body \
                return true; \
            })) return false; \
    } while (false)

#define MG_INCLUDE(path) \
    do { \
        if (!inspector.include(path)) return false; \
    } while (false)

#define MG_CONTROL(widget, ...) \
    do { \
        if (!inspector.control(widget, __VA_ARGS__)) return false; \
    } while (false)

#define MG_CONTROL_IF(widget, visible_when, ...) \
    do { \
        if (!inspector.control(widget, __VA_ARGS__, visible_when)) return false; \
    } while (false)

#define MG_IN(path, ...) ::mg::inspector::Rule{path, "in", {__VA_ARGS__}}
#define MG_BIND(name, path) ::mg::inspector::Binding{name, path}
#else
#define MG_INSPECTOR_BEGIN()
#define MG_INSPECTOR_END()
#define MG_SECTION(title, body)
#define MG_ROW(body)
#define MG_LABELED_ROW(label, body)
#define MG_PROPERTY(path, label, widget)
#define MG_PROPERTY_IF(path, label, widget, visible_when)
#define MG_INCLUDE(path)
#define MG_CONTROL(widget, ...)
#define MG_CONTROL_IF(widget, visible_when, ...)
#define MG_IN(path, ...)
#define MG_BIND(name, path)
#endif
