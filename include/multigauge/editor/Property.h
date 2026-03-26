#pragma once

#include <rapidjson/document.h>
#include <multigauge/editor/EnumTraits.h>

class PropertyObject;

struct PropertyMetadata {
    using OptionsGetter = rapidjson::Value (*)(rapidjson::Document::AllocatorType&);
    using TypeListGetter = rapidjson::Value (*)(rapidjson::Document::AllocatorType&);
    using RuleListGetter = rapidjson::Value (*)(rapidjson::Document::AllocatorType&);

    /// @brief Display name used in editor
    const char* name = nullptr;
    /// @brief Brief description used in editor
    const char* description = nullptr;
    /// @brief Widget type to display for UI in editor
    const char* widget = nullptr;
    /// @brief Whether the editor should treat this property as nullable.
    bool nullable = false;
    /// @brief Declarative visibility rules for the editor UI.
    RuleListGetter getVisibleWhen = nullptr;
    /// @brief Declarative interactability rules for the editor UI.
    RuleListGetter getInteractableWhen = nullptr;
    /// @brief Returns dropdown/select options metadata when available.
    OptionsGetter getOptions = nullptr;
    /// @brief Returns available polymorphic types metadata when available.
    TypeListGetter getTypes = nullptr;
};

struct Property {
    using Setter = bool (*)(PropertyObject*, const rapidjson::Value&);
    using Getter = bool (*)(const PropertyObject*, rapidjson::Value&, rapidjson::Document::AllocatorType&);
    using ChildGetter = const PropertyObject* (*)(const PropertyObject*);
    using ChildGetterMutable = PropertyObject* (*)(PropertyObject*);

    /// @brief Key used in JSON schema
    const char* key = nullptr;
    /// @brief Setter function using json value as input
    Setter set = nullptr;
    /// @brief Getter function using json value as output
    Getter get = nullptr;
    /// @brief Returns nested PropertyObject for group-like properties, or nullptr.
    ChildGetter getChild = nullptr;
    /// @brief Returns mutable nested PropertyObject for group-like properties, or nullptr.
    ChildGetterMutable getChildMutable = nullptr;
    const PropertyMetadata meta;

    /// @brief Returns a JSON object containing metadata about this property.
    rapidjson::Value getBaseMeta(rapidjson::Document::AllocatorType& a) const;
};
