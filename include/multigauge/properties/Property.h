#pragma once

#include <multigauge/json/Json.h>

#include <multigauge/properties/EnumTraits.h>
#include <multigauge/Config.h>
#if MG_BUILD_EDITOR
#include <multigauge/properties/meta/PropertyMetadata.h>
#endif

namespace mg {

class PropertyObject; // Forward declaration

/// @brief Describes a property exposed through the property system.
struct Property {
    /// @brief Function used to decode and assign a property value.
    using Setter = bool (*)(PropertyObject*, json::Reader);

    /// @brief Function used to encode a property value.
    using Getter = bool (*)(const PropertyObject*, json::Writer&);

    /// @brief Function used to access a nested PropertyObject.
    using ChildGetter = const PropertyObject* (*)(const PropertyObject*);

    const char* key = nullptr; ///< JSON key.
    Setter set = nullptr; ///< Setter function using json value as input.
    Getter get = nullptr; ///< Getter function using json value as output.
    /// Returns a borrowed nested property object, or nullptr when absent.
    /// The returned object must be owned by the supplied property object.
    ChildGetter getChild = nullptr;

#if MG_BUILD_EDITOR
    const PropertyMetadata meta; ///< Property metadata.

    bool writeBaseMeta(json::ObjectWriter& object) const;
#endif
};

} // namespace mg
