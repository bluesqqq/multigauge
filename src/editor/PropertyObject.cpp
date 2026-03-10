#include <multigauge/editor/PropertyObject.h>

namespace {
    static PropertyObject::PropertyList nextPropertyList(
        const PropertyObject* self,
        PropertyObject::PropertyList current
    ) {
        if (!current.parent) return {nullptr, 0, nullptr};
        return current.parent(self);
    }
}

const Property *PropertyObject::findProperty(const char *key) const {
    if (!key) return nullptr;

    for (PropertyList pl = propertyList(); pl.props || pl.parent; pl = nextPropertyList(this, pl)) {
        if (!pl.props || pl.count == 0) continue;

        for (std::size_t i = 0; i < pl.count; ++i) {
            const char* propName = pl.props[i].key;
            if (propName && std::strcmp(propName, key) == 0)
                return &pl.props[i];
        }
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

    for (PropertyList pl = propertyList(); pl.props || pl.parent; pl = nextPropertyList(this, pl)) {
        if (!pl.props || pl.count == 0) continue;

        for (std::size_t i = 0; i < pl.count; ++i) {
            const Property& property = pl.props[i];
            if (!property.key || !property.get) continue;

            // Skip shadowed base properties when a derived class reuses the same key.
            if (findProperty(property.key) != &property) continue;

            rapidjson::Value val;
            if (!property.get(this, val, a)) continue;

            out.AddMember(rapidjson::StringRef(property.key), val, a);
        }
    }
}

rapidjson::Value PropertyObject::getPropertiesMeta(rapidjson::Document::AllocatorType &a) const {
    rapidjson::Value metas(rapidjson::kArrayType);

    for (auto pl = propertyList(); pl.props || pl.parent; pl = nextPropertyList(this, pl)) {
        if (!pl.props || pl.count == 0) continue;

        for (std::size_t i = 0; i < pl.count; ++i) {
            const auto& prop = pl.props[i];
            metas.PushBack(prop.getMeta(this, a), a);
        }
    }

    return metas;
}

std::vector<const Property*> PropertyObject::getPropertyObjectProperties() const {
    std::vector<const Property*> result;

    for (PropertyList pl = propertyList(); pl.props || pl.parent; pl = nextPropertyList(this, pl)) {
        if (!pl.props || pl.count == 0) continue;

        for (std::size_t i = 0; i < pl.count; ++i) {
            const auto& property = pl.props[i];
            if (!property.key || !property.get) continue;

            // Skip shadowed base properties when a derived class reuses the same key.
            if (findProperty(property.key) != &property) continue;

            // Check if property is PropertyObject class
            if (false) result.push_back(&property);
        }
    }

    return result;
}

