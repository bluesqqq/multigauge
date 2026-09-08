#pragma once

#include <multigauge/json/Json.h>

namespace mg {

class PropertyObject; // Forward declaration

/// @brief Semantic constraints for a property exposed through the property system.
struct PropertyMetadata {
    using OptionsGetter = bool (*)(json::Writer&);
    using TypeListGetter = bool (*)(json::Writer&);

    bool nullable = false; ///< Whether JSON null is a valid property value.
    OptionsGetter getOptions = nullptr; ///< Retrieves the finite set of valid values, when applicable.
    TypeListGetter getTypes = nullptr; ///< Retrieves valid concrete types for a polymorphic property.
};

}
