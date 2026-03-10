#pragma once

#include <rapidjson/document.h>

class PropertyObject;

struct Property {
    using Setter = bool (*)(PropertyObject*, const rapidjson::Value&);
    using Getter = bool (*)(const PropertyObject*, rapidjson::Value&, rapidjson::Document::AllocatorType&);
    
    //----------[ JSON ]----------//

    /// @brief Key used in JSON schema
    const char* key;

    //----------[ METADATA ]----------//
    
    /// @brief Display name used in editor
    const char* name;
    /// @brief Brief description used in editor
    const char* description;
    /// @brief Widget type to display for UI in editor
    //const char* type;

    rapidjson::Value getMeta(rapidjson::Document::AllocatorType& a) const;

    //----------[ HANDLERS ]----------//

    /// @brief Setter function using json value as input
    Setter set;
    /// @brief Getter function using json value as output
    Getter get;
};