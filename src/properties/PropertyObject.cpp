#include <multigauge/properties/PropertyObject.h>

namespace mg {

const Property* PropertyObject::findProperty(std::string_view key) const {
    if (key.empty()) return nullptr;

    const Property* found = nullptr;

    propertyList().forEach(this, [&](const Property& prop) {
        if (found) return;

        const char* propName = prop.key;
        if (propName && key == propName) {
            found = &prop;
        }
    });

    return found;
}

bool PropertyObject::setProperty(const char* key, const rapidjson::Value& v) {
    if (!key) return false;
    const Property* property = findProperty(std::string_view(key));
    if (!property || !property->set) return false;
    return property->set(this, v);
}

bool PropertyObject::getProperty(const char* key, rapidjson::Value& out, rapidjson::Document::AllocatorType& a) const {
    if (!key) return false;
    const Property* property = findProperty(std::string_view(key));
    if (!property || !property->get) return false;
    return property->get(this, out, a);
}

bool PropertyObject::loadProperties(rapidjson::Value::ConstObject json) {
    bool success = true;
    for (auto it = json.MemberBegin(); it != json.MemberEnd(); ++it) {
        const Property* property = findProperty(std::string_view(it->name.GetString(), it->name.GetStringLength()));
        if (property && property->set && !property->set(this, it->value)) {
            success = false;
        }
    }
    return success;
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

#if MG_ENABLE_EDITOR_REFLECTION
    propertyList().forEach(this, [&](const Property& prop) {
        if (!prop.key) return;
        if (findProperty(prop.key) != &prop) return;
        if (!prop.meta.inspectorVisible) return;
        metas.PushBack(getPropertyMeta(prop, a), a);
    });
#endif

    return metas;
}

rapidjson::Value PropertyObject::getPropertyMeta(const Property& prop, rapidjson::Document::AllocatorType& a) const {
#if MG_ENABLE_EDITOR_REFLECTION
    rapidjson::Value meta = prop.getBaseMeta(a);

    if (prop.getChildConst) {
        if (prop.meta.getTypesMeta) {
            rapidjson::Value types(rapidjson::kObjectType);
            const PropertyObject* child = prop.getChildConst(this);

            if (child && child->typeId()) {
                types.AddMember("current", rapidjson::Value(child->typeId(), a), a);
            } else {
                types.AddMember("current", rapidjson::Value(rapidjson::kNullType), a);
            }

            types.AddMember("all", prop.meta.getTypesMeta(a), a);
            meta.AddMember("types", std::move(types), a);
        }

        rapidjson::Value props(rapidjson::kArrayType);
        const PropertyObject* child = prop.getChildConst(this);
        if (child) {
            props = child->getPropertiesMeta(a);
        }

        meta.AddMember("properties", props, a);
        return meta;
    }

    rapidjson::Value value;
    if (prop.get && prop.get(this, value, a)) {
        meta.AddMember("value", std::move(value), a);
    } else {
        meta.AddMember("value", rapidjson::Value(rapidjson::kNullType), a);
    }

    return meta;
#else
    (void)prop;
    (void)a;
    return rapidjson::Value(rapidjson::kObjectType);
#endif
}

bool PropertyObject::resolvePath(const std::string& path, PropertyObject*& owner, const Property*& prop) {
    owner = nullptr;
    prop = nullptr;

    PropertyObject* current = this;
    std::string_view remaining(path);

    while (!remaining.empty()) {
        const std::size_t dot = remaining.find('.');
        const std::string_view segment = remaining.substr(0, dot);
        if (segment.empty()) {
            if (dot == std::string_view::npos) return false;
            remaining.remove_prefix(dot + 1);
            continue;
        }

        const Property* p = current->findProperty(segment);
        if (!p) return false;

        const std::string_view next = dot == std::string_view::npos ? std::string_view{} : remaining.substr(dot + 1);
        if (next.find_first_not_of('.') == std::string_view::npos) {
            owner = current;
            prop = p;
            return true;
        }

        if (!p->getChild) return false;

        current = p->getChild(current);
        if (!current) return false;
        remaining = next;
    }

    return false;
}

bool PropertyObject::resolvePath(const std::string& path, const PropertyObject*& owner, const Property*& prop) const {
    owner = nullptr;
    prop = nullptr;

    const PropertyObject* current = this;
    std::string_view remaining(path);

    while (!remaining.empty()) {
        const std::size_t dot = remaining.find('.');
        const std::string_view segment = remaining.substr(0, dot);
        if (segment.empty()) {
            if (dot == std::string_view::npos) return false;
            remaining.remove_prefix(dot + 1);
            continue;
        }

        const Property* p = current->findProperty(segment);
        if (!p) return false;

        const std::string_view next = dot == std::string_view::npos ? std::string_view{} : remaining.substr(dot + 1);
        if (next.find_first_not_of('.') == std::string_view::npos) {
            owner = current;
            prop = p;
            return true;
        }

        if (!p->getChildConst) return false;

        current = p->getChildConst(current);
        if (!current) return false;
        remaining = next;
    }

    return false;
}

} // namespace mg
