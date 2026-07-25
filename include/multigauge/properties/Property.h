#pragma once

#include <rapidjson/document.h>

#include <multigauge/Config.h>
#include <multigauge/properties/EnumTraits.h>
#include <multigauge/properties/meta/Rules.h>
#if MG_ENABLE_EDITOR_REFLECTION
#include <multigauge/properties/meta/PropertyMetadata.h>
#endif

namespace mg {

class PropertyObject;

struct Property {
    using Setter = bool (*)(PropertyObject*, const rapidjson::Value&);
    using Getter = bool (*)(const PropertyObject*, rapidjson::Value&, rapidjson::Document::AllocatorType&);
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

    rapidjson::Value getBaseMeta(rapidjson::Document::AllocatorType& a) const;
#endif
};

} // namespace mg
