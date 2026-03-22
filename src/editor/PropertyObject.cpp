#include <multigauge/editor/PropertyObject.h>

const Property* PropertyObject::findProperty(const char* key) const {
    if (!key) return nullptr;

    const Property* found = nullptr;

    propertyList().forEach(this, [&](const Property& prop) {
        if (found) return;

        const char* propName = prop.key;
        if (propName && std::strcmp(propName, key) == 0) {
            found = &prop;
        }
    });

    return found;
}

bool PropertyObject::loadProperty(const char* key, const rapidjson::Value& v) {
    const Property* property = findProperty(key);
    if (!property || !property->set) return false;
    return property->set(this, v);
}

bool PropertyObject::saveProperty(const char* key, rapidjson::Value& out, rapidjson::Document::AllocatorType& a) const {
    const Property* property = findProperty(key);
    if (!property || !property->get) return false;
    return property->get(this, out, a);
}

void PropertyObject::loadProperties(rapidjson::Value::ConstObject json) {
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it) {
        const char* key = it->name.GetString();
        loadProperty(key, it->value);
    }
}

void PropertyObject::saveProperties(rapidjson::Value& out, rapidjson::Document::AllocatorType& a) const {
    out.SetObject();

    const char* type = typeId();
    if (type) {
        out.AddMember(rapidjson::Value(TYPE_KEY, a), rapidjson::Value(type, a), a);
    }

    propertyList().forEach(this, [&](const Property& property) {
        if (!property.key || !property.get) return;
        if (findProperty(property.key) != &property) return;

        rapidjson::Value val;
        if (!property.get(this, val, a)) return;

        out.AddMember(rapidjson::StringRef(property.key), val, a);
    });
}

rapidjson::Value PropertyObject::getPropertiesMeta(rapidjson::Document::AllocatorType& a) const {
    rapidjson::Value metas(rapidjson::kArrayType);

    propertyList().forEach(this, [&](const Property& prop) {
        if (!prop.key) return;
        if (findProperty(prop.key) != &prop) return;
        metas.PushBack(getPropertyMeta(prop, a), a);
    });

    return metas;
}

rapidjson::Value PropertyObject::getPropertyMeta(const Property& prop, rapidjson::Document::AllocatorType& a) const {
    rapidjson::Value meta = prop.getBaseMeta(a);

    if (prop.getChild) {
        meta.AddMember("nullable", false, a);

        rapidjson::Value props(rapidjson::kArrayType);
        const PropertyObject* child = prop.getChild(this);
        if (child) {
            props = child->getPropertiesMeta(a);
        }

        meta.AddMember("properties", props, a);
        return meta;
    }

    rapidjson::Value value;
    if (prop.get && prop.get(this, value, a)) {
        meta.AddMember("nullable", false, a);
        meta.AddMember("value", value, a);
    } else {
        meta.AddMember("nullable", false, a);
        meta.AddMember("value", rapidjson::Value(rapidjson::kNullType), a);
    }

    return meta;
}

std::vector<std::string> PropertyObject::splitPath(const std::string& path) {
    std::vector<std::string> parts;
    std::string current;

    for (char c : path) {
        if (c == '.') {
            if (!current.empty()) {
                parts.push_back(current);
                current.clear();
            }
        } else {
            current.push_back(c);
        }
    }

    if (!current.empty()) {
        parts.push_back(current);
    }

    return parts;
}

bool PropertyObject::resolvePath(const std::string& path, PropertyObject*& owner, const Property*& prop) {
    owner = nullptr;
    prop = nullptr;

    const auto parts = splitPath(path);
    if (parts.empty()) return false;

    PropertyObject* current = this;

    for (std::size_t i = 0; i < parts.size(); ++i) {
        const bool last = (i + 1 == parts.size());

        const Property* p = current->findProperty(parts[i].c_str());
        if (!p) return false;

        if (last) {
            owner = current;
            prop = p;
            return true;
        }

        if (!p->getChildMutable) return false;

        current = p->getChildMutable(current);
        if (!current) return false;
    }

    return false;
}

bool PropertyObject::resolvePath(const std::string& path, const PropertyObject*& owner, const Property*& prop) const {
    owner = nullptr;
    prop = nullptr;

    const auto parts = splitPath(path);
    if (parts.empty()) return false;

    const PropertyObject* current = this;

    for (std::size_t i = 0; i < parts.size(); ++i) {
        const bool last = (i + 1 == parts.size());

        const Property* p = current->findProperty(parts[i].c_str());
        if (!p) return false;

        if (last) {
            owner = current;
            prop = p;
            return true;
        }

        if (!p->getChild) return false;

        current = p->getChild(current);
        if (!current) return false;
    }

    return false;
}
