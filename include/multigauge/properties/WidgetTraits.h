#pragma once

#include <memory>
#include <optional>
#include <string>
#include <concepts>
#include <type_traits>
#include <vector>

#include <multigauge/properties/EnumTraits.h>

namespace mg {

namespace graphics {
    struct rgba;
}

class PropertyObject;

template <typename T, typename = void>
struct MgPropWidgetTraits {
    static constexpr const char* value = "json";
};

template <typename T>
struct MgPropNullableTraits {
    static constexpr bool value = false;
};

template <typename T>
    requires EnumDescribed<EnumTraitsTypeT<T>>
struct MgPropWidgetTraits<T> {
    static constexpr const char* value = "select";
};

#define MG_EDITOR_WIDGET(type, widget_literal) \
template <> \
struct MgPropWidgetTraits<type> { static constexpr const char* value = widget_literal; };

MG_EDITOR_WIDGET(bool, "boolean")
MG_EDITOR_WIDGET(graphics::rgba, "color-selector")

template <typename T>
    requires (std::is_arithmetic_v<T> && !std::is_same_v<T, bool>)
struct MgPropWidgetTraits<T> {
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
    requires std::derived_from<T, PropertyObject>
struct MgPropWidgetTraits<T> {
    static constexpr const char* value = "group";
};

template <typename T>
struct MgPropWidgetTraits<std::unique_ptr<T>> {
    static constexpr const char* value = MgPropWidgetTraits<T>::value;
};

} // namespace mg
