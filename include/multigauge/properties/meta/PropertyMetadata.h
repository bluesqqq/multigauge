#pragma once

#include <multigauge/json/Json.h>
#include <multigauge/properties/meta/Rules.h>

namespace mg {

class PropertyObject; // Forward declaration

/// @brief Editor-facing metadata for a property exposed through the property system.
struct PropertyMetadata {
    using OptionsGetter = bool (*)(json::Writer&);
    using TypeListGetter = bool (*)(json::Writer&);
    using RuleListGetter = bool (*)(json::Writer&);

    //----------[ GENERIC ]----------//

    const char* name = nullptr;        ///< Display name shown in the editor.
    const char* description = nullptr; ///< Brief description shown in the editor.
    const char* widget = nullptr;      ///< Editor widget used to edit the property.

    //----------[ INTERACTION ]----------//

    bool nullable = false;                  ///< Whether the property may be set to null.
    bool inspectorVisible = true;           ///< Whether the property is shown in the editor inspector.
    RuleListGetter getVisibleWhen = nullptr;      ///< Retrieves rules controlling property visibility.
    RuleListGetter getInteractableWhen = nullptr; ///< Retrieves rules controlling property interactability.

    //----------[ SELECTION ]----------//

    OptionsGetter getOptionsMeta = nullptr; ///< Retrieves metadata for available selection options.
    TypeListGetter getTypesMeta = nullptr;  ///< Retrieves metadata for available polymorphic types.
};

}
