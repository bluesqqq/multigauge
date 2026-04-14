#pragma once

#include <rapidjson/document.h>
#include <multigauge/properties/EnumTraits.h>
#include <multigauge/properties/rules/Rules.h>

class PropertyObject;

struct PropertyMetadata {
    using OptionsGetter = rapidjson::Value (*)(rapidjson::Document::AllocatorType&);
    using TypeListGetter = rapidjson::Value (*)(rapidjson::Document::AllocatorType&);
    using RuleListGetter = rapidjson::Value (*)(rapidjson::Document::AllocatorType&);

    //----------[ GENERIC ]----------//

    /// Display name used in editor
    const char* name = nullptr;
    ///  Brief description used in editor
    const char* description = nullptr;
    /// Widget type to display for UI in editor
    const char* widget = nullptr;

    //----------[ INTERACTION ]----------//

    /// Whether the editor should treat this property as nullable.
    bool nullable = false;
    /// Whether this property should be listed in inspector metadata.
    bool inspectorVisible = true;
    /// Declarative visibility rules for the editor UI.
    RuleListGetter getVisibleWhen = nullptr;
    /// Declarative interactability rules for the editor UI.
    RuleListGetter getInteractableWhen = nullptr;

    //----------[  ]----------//

    /// Returns dropdown/select options metadata when available.
    OptionsGetter getOptions = nullptr;
    /// Returns available polymorphic types metadata when available.
    TypeListGetter getTypes = nullptr;
};

struct Property {
    using Setter = bool (*)(PropertyObject*, const rapidjson::Value&);
    using Getter = bool (*)(const PropertyObject*, rapidjson::Value&, rapidjson::Document::AllocatorType&);
    using ChildGetter = const PropertyObject* (*)(const PropertyObject*);
    using ChildGetterMutable = PropertyObject* (*)(PropertyObject*);

    /// Key used in JSON schema
    const char* key = nullptr;
    /// Setter function using json value as input
    Setter set = nullptr;
    /// Getter function using json value as output
    Getter get = nullptr;
    /// Returns nested PropertyObject for group-like properties, or nullptr.
    ChildGetter getChild = nullptr;
    ///  Returns mutable nested PropertyObject for group-like properties, or nullptr.
    ChildGetterMutable getChildMutable = nullptr;
    const PropertyMetadata meta;

    rapidjson::Value getBaseMeta(rapidjson::Document::AllocatorType& a) const;
};
