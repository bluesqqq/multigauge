#pragma once

#include <multigauge/json/Json.h>

namespace mg {

class PropertyObject; // Forward declaration

/// Editor-only capabilities and metadata for a collection property.
struct CollectionMetadata {
    using ItemsGetter = bool (*)(const PropertyObject*, json::Writer&);
    using DefaultGetter = bool (*)(const PropertyObject*, json::Writer&);

    /// Applies a serialized collection operation.
    ///
    /// The operation is an object with an `action` member. Supported actions are
    /// `append`, `remove`, and `update`; inspector metadata lists the actions
    /// available for a particular property.
    using Mutator = bool (*)(PropertyObject*, json::Reader);

    ItemsGetter getItems = nullptr;
    DefaultGetter getDefault = nullptr;
    Mutator mutate = nullptr;
};

/// @brief Semantic constraints for a property exposed through the property system.
struct PropertyMetadata {
    using Validator = bool (*)(const PropertyObject*, json::Reader);
    using OptionsGetter = bool (*)(json::Writer&);
    using TypeListGetter = bool (*)(json::Writer&);
    using DefaultGetter = bool (*)(json::Writer&);

    bool nullable = false; ///< Whether JSON null is a valid property value.
    Validator validate = nullptr; ///< Validates a serialized value without assigning it.
    OptionsGetter getOptions = nullptr; ///< Retrieves the finite set of valid values, when applicable.
    TypeListGetter getTypes = nullptr; ///< Retrieves valid concrete types for a polymorphic property.
    DefaultGetter getDefault = nullptr; ///< Retrieves the default serialized value, when available.
    const CollectionMetadata* collection = nullptr; ///< Editor-only collection capabilities, when applicable.
};

}
