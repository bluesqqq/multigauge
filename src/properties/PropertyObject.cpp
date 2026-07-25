#include <multigauge/properties/PropertyObject.h>

#include <cstring>

namespace mg {
namespace {
bool loadMember(void* context, std::string_view key, json::Reader value) {
    auto* object = static_cast<PropertyObject*>(context);
    const Property* property = object->findProperty(key);
    return !property || !property->set || property->set(object, value);
}
}

const Property* PropertyObject::findProperty(std::string_view key) const {
    if (key.empty()) return nullptr;
    const Property* found = nullptr;
    propertyList().forEach(this, [&](const Property& prop) {
        if (!found && prop.key && key == prop.key) found = &prop;
    });
    return found;
}

bool PropertyObject::setProperty(std::string_view key, json::Reader value) {
    const Property* property = findProperty(key);
    return property && property->set && property->set(this, value);
}

bool PropertyObject::getProperty(std::string_view key, json::Writer& writer) const {
    const Property* property = findProperty(key);
    return property && property->get && property->get(this, writer);
}

bool PropertyObject::loadProperties(json::Reader object) {
    return object.isObject() && object.forEachMember(&loadMember, this);
}

bool PropertyObject::saveProperties(json::Writer& writer) const {
    return writer.writeObject([&](json::ObjectWriter& object) {
        if (const char* type = typeId(); type && !object.write(TYPE_KEY, type)) return false;
        bool success = true;
        propertyList().forEach(this, [&](const Property& property) {
            if (!success || !property.key || !property.get || findProperty(property.key) != &property) return;
            success = object.writeValue(property.key, [&](json::Writer& value) { return property.get(this, value); });
        });
        return success;
    });
}

bool PropertyObject::writePropertiesMeta(json::Writer& writer) const {
    return writer.writeArray([&](json::ArrayWriter& array) {
        bool success = true;
        propertyList().forEach(this, [&](const Property& property) {
            if (!success || !property.key || !property.meta.inspectorVisible || findProperty(property.key) != &property) return;
            success = writePropertyMeta(array.writer(), property);
        });
        return success;
    });
}

bool PropertyObject::writePropertyMeta(json::Writer& writer, const Property& prop) const {
    return writer.writeObject([&](json::ObjectWriter& object) {
        if (!prop.writeBaseMeta(object)) return false;
        if (prop.getChildConst) {
            if (prop.meta.getTypesMeta && !object.writeObject("types", [&](json::ObjectWriter& types) {
                const PropertyObject* child = prop.getChildConst(this);
                if (child && child->typeId()) { if (!types.write("current", child->typeId())) return false; }
                else if (!types.writeValue("current", [](json::Writer& value) { return value.null(); })) return false;
                return types.writeValue("all", prop.meta.getTypesMeta);
            })) return false;
            const PropertyObject* child = prop.getChildConst(this);
            return object.writeArray("properties", [&](json::ArrayWriter& properties) {
                if (!child) return true;
                return child->writePropertiesMeta(properties.writer());
            });
        }
        return object.writeValue("value", [&](json::Writer& value) { return prop.get ? prop.get(this, value) : value.null(); });
    });
}

std::vector<std::string> PropertyObject::splitPath(const std::string& path) {
    std::vector<std::string> parts; std::string current;
    for (char c : path) { if (c == '.') { if (!current.empty()) { parts.push_back(std::move(current)); current.clear(); } } else current.push_back(c); }
    if (!current.empty()) parts.push_back(std::move(current));
    return parts;
}

bool PropertyObject::resolvePath(const std::string& path, PropertyObject*& owner, const Property*& prop) {
    owner = nullptr; prop = nullptr; const auto parts = splitPath(path); if (parts.empty()) return false; PropertyObject* current = this;
    for (std::size_t i = 0; i < parts.size(); ++i) { const Property* found = current->findProperty(parts[i].c_str()); if (!found) return false; if (i + 1 == parts.size()) { owner = current; prop = found; return true; } if (!found->getChild || !(current = found->getChild(current))) return false; }
    return false;
}

bool PropertyObject::resolvePath(const std::string& path, const PropertyObject*& owner, const Property*& prop) const {
    owner = nullptr; prop = nullptr; const auto parts = splitPath(path); if (parts.empty()) return false; const PropertyObject* current = this;
    for (std::size_t i = 0; i < parts.size(); ++i) { const Property* found = current->findProperty(parts[i].c_str()); if (!found) return false; if (i + 1 == parts.size()) { owner = current; prop = found; return true; } if (!found->getChildConst || !(current = found->getChildConst(current))) return false; }
    return false;
}

} // namespace mg
