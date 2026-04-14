#pragma once

#include <rapidjson/document.h>
#include <multigauge/properties/meta/Rules.h>

namespace mg {

class PropertyObject;

/// Editor-facing metadata for a property exposed through the property system
/// 
/// Describes how a property should appear and behave in tooling. This currently
/// includes display labels, widget types, optional choices, and conditional UI rules.
///
/// Metadata does not store the property's runtime value and does not enforce runtime
/// behavior. It is only used by editors/tooling that require it for interfaces.
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

}
