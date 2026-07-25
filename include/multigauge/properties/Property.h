#pragma once

#include <multigauge/json/Json.h>

#include <multigauge/properties/EnumTraits.h>
#include <multigauge/Config.h>
#if MG_ENABLE_EDITOR_REFLECTION
#include <multigauge/properties/meta/PropertyMetadata.h>
#endif

namespace mg {

class PropertyObject;

struct Property {
    using Setter = bool (*)(PropertyObject*, json::Reader);
    using Getter = bool (*)(const PropertyObject*, json::Writer&);
    using ChildGetter = PropertyObject* (*)(PropertyObject*);
    using ChildGetterConst = const PropertyObject* (*)(const PropertyObject*);

    /// Key used in JSON schema
    const char* key = nullptr;
    /// Setter function using json value as input
    Setter set = nullptr;
    /// Getter function using json value as output
    Getter get = nullptr;
    /// Returns mutable nested PropertyObject for group-like properties, or nullptr.
    ChildGetter getChild = nullptr;
    /// Returns const nested PropertyObject for group-like properties, or nullptr.
    ChildGetterConst getChildConst = nullptr;
#if MG_ENABLE_EDITOR_REFLECTION
    const PropertyMetadata meta;

    bool writeBaseMeta(json::ObjectWriter& object) const;
#endif
};

} // namespace mg
