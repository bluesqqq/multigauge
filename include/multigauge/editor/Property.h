#pragma once

#include <rapidjson/document.h>

class PropertyObject;

struct Property {
    using Setter = bool (*)(PropertyObject*, const rapidjson::Value&);
    using Getter = bool (*)(const PropertyObject*, rapidjson::Value&, rapidjson::Document::AllocatorType&);
    
    using ChildGetter = const PropertyObject* (*)(const PropertyObject*);
    using ChildGetterMutable = PropertyObject* (*)(PropertyObject*);

    //----------[ JSON ]----------//

    /// @brief Key used in JSON schema
    const char* key = nullptr;

    //----------[ METADATA ]----------//
    
    /// @brief Display name used in editor
    const char* name = nullptr;
    /// @brief Brief description used in editor
    const char* description = nullptr;
    /// @brief Widget type to display for UI in editor
    const char* widget = nullptr;

    //----------[ HANDLERS ]----------//

    /// @brief Setter function using json value as input
    Setter set = nullptr;
    /// @brief Getter function using json value as output
    Getter get = nullptr;

    /// @brief Returns nested PropertyObject for group-like properties, or nullptr.
    ChildGetter getChild = nullptr;

    /// @brief Returns mutable nested PropertyObject for group-like properties, or nullptr.
    ChildGetterMutable getChildMutable = nullptr;

    /// @brief Returns a JSON object containing metadata about this property.
    rapidjson::Value getBaseMeta(rapidjson::Document::AllocatorType& a) const;
};