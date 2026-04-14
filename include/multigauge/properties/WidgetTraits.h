#pragma once

#include <memory>
#include <optional>
#include <string>
#include <type_traits>
#include <vector>

#include <multigauge/properties/EnumTraits.h>

struct rgba;
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
