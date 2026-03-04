#pragma once

#include <rapidjson/document.h>

class Editable;

struct Property {
    //----------[ JSON ]----------//

    /// @brief Key used in JSON schema
    const char* key;

    //----------[ EDITOR ]----------//
    
    /// @brief Display name used in editor
    const char* name;
    /// @brief Brief description used in editor
    const char* description;
    /// @brief Widget type to display for UI in editor
    //const char* type;

    //----------[ HANDLERS ]----------//

    /// @brief Setter function using json value as input
    bool (*set)(Editable* obj, const rapidjson::Value& v);
    /// @brief Getter function using json value as output
    bool (*get)(const Editable* obj, rapidjson::Value& out, rapidjson::Document::AllocatorType& a);
};