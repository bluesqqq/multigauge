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

bool Editable::loadProperty(const char *key, const rapidjson::Value &v){
    auto property = findProperty(key);
    if (!property || !property->set) return false;
    return property->set(this, v);
}

bool Editable::saveProperty(const char *key, rapidjson::Value &out, rapidjson::Document::AllocatorType &a) {
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
    for (std::size_t i = 0; i < pl.count; ++i) {
        const Property& p = pl.props[i];
        if (!p.key || !p.get) continue;

        rapidjson::Value key;
        key.SetString(p.key, a);

        rapidjson::Value val;
        if (!p.get(this, val, a)) continue;

        out.AddMember(key, val, a);
    }
}