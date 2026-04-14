#pragma once

#include <rapidjson/document.h>

#include <multigauge/properties/EnumTraits.h>
#include <multigauge/properties/meta/PropertyMetadata.h>

class PropertyObject;

struct Property {
    using Setter = bool (*)(PropertyObject*, const rapidjson::Value&);
    using Getter = bool (*)(const PropertyObject*, rapidjson::Value&, rapidjson::Document::AllocatorType&);
    using ChildGetter = const PropertyObject* (*)(const PropertyObject*);
    using ChildGetterMutable = PropertyObject* (*)(PropertyObject*);

    /// Key used in JSON schema
    const char* key = nullptr;
    /// Setter function using json value as input
    Setter set = nullptr;
    /// Getter function using json value as output
    Getter get = nullptr;
    /// Returns nested PropertyObject for group-like properties, or nullptr.
    ChildGetter getChild = nullptr;
    ///  Returns mutable nested PropertyObject for group-like properties, or nullptr.
    ChildGetterMutable getChildMutable = nullptr;
    const mg::PropertyMetadata meta;

    rapidjson::Value getBaseMeta(rapidjson::Document::AllocatorType& a) const;
};
