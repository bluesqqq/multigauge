#include <multigauge/editor/Editable.h>

const Property *Editable::findProperty(const char *key) const {
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

bool Editable::loadProperty(const char *key, const rapidjson::Value &v) {
    auto property = findProperty(key);
    if (!property || !property->set) return false;
    return property->set(this, v);
}

bool Editable::saveProperty(const char *key, rapidjson::Value &out, rapidjson::Document::AllocatorType &a) const {
    auto property = findProperty(key);
    if (!property || !property->get) return false;
    return property->get(this, out, a);
}

void Editable::loadProperties(rapidjson::Value::ConstObject json) {
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it) {
        const char* key = it->name.GetString();
        loadProperty(key, it->value);
    }
}

void Editable::saveProperties(rapidjson::Value &out, rapidjson::Document::AllocatorType &a) const {
    out.SetObject();

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

std::vector<const Property*> Editable::getEditableProperties() const {
    PropertyList pl = propertyList();
    if (!pl.props || pl.count == 0) return {};

    std::vector<const Property*> result;

    for (std::size_t i = 0; i < pl.count; ++i) {
        const auto& property = pl.props[i];
        if (!property.key || !property.get) continue;

        // Check if property is Editable class
        if (false) result.push_back(&property);
    }

    return result;
}