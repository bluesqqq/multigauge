#include <multigauge/editor/PropertyObject.h>

const Property *PropertyObject::findProperty(const char *key) const {
    if (!key) return nullptr;

    PropertyList pl = propertyList();
    if (!pl.props || pl.count == 0) return nullptr;

    for (std::size_t i = 0; i < pl.count; ++i) {
        const char* propName = pl.props[i].key;
        if (propName && std::strcmp(propName, key) == 0)
            return &pl.props[i];
    }

    return nullptr;
}

bool PropertyObject::loadProperty(const char *key, const rapidjson::Value &v) {
    auto property = findProperty(key);
    if (!property || !property->set) return false;
    return property->set(this, v);
}

bool PropertyObject::saveProperty(const char *key, rapidjson::Value &out, rapidjson::Document::AllocatorType &a) const {
    auto property = findProperty(key);
    if (!property || !property->get) return false;
    return property->get(this, out, a);
}

void PropertyObject::loadProperties(rapidjson::Value::ConstObject json) {
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it) {
        const char* key = it->name.GetString();
        loadProperty(key, it->value);
    }
}

void PropertyObject::saveProperties(rapidjson::Value &out, rapidjson::Document::AllocatorType &a) const {
    out.SetObject();

    // Add type if it exists for this object
    const char* type = typeId();
    if (type) out.AddMember(rapidjson::Value(TYPE_KEY, a), rapidjson::Value(type, a), a);

    PropertyList pl = propertyList();
    if (!pl.props || pl.count == 0) return;

    for (std::size_t i = 0; i < pl.count; ++i) {
        const Property& property = pl.props[i];
        if (!property.key || !property.get) continue;

        rapidjson::Value val;
        if (!property.get(this, val, a)) continue;

        out.AddMember(rapidjson::StringRef(property.key), val, a);
    }
}

std::vector<const Property*> PropertyObject::getPropertyObjectProperties() const {
    PropertyList pl = propertyList();
    if (!pl.props || pl.count == 0) return {};

    std::vector<const Property*> result;

    for (std::size_t i = 0; i < pl.count; ++i) {
        const auto& property = pl.props[i];
        if (!property.key || !property.get) continue;

        // Check if property is PropertyObject class
        if (false) result.push_back(&property);
    }

    return result;
}