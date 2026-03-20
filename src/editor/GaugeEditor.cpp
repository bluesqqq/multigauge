#include <multigauge/editor/GaugeEditor.h>

std::string GaugeEditor::toString(const rapidjson::Value& v) {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);
    v.Accept(w);
    return std::string(sb.GetString(), sb.GetSize());
}

void GaugeEditor::indexElementRecursive(Element& e, Id parentId, std::uint32_t order) {
    const Id id = nextId++;
    idToPtr[id] = &e;
    ptrToId[&e] = id;

    EditorNode node;
    node.id = id;
    node.parentId = parentId;
    node.order = order;
    node.type = e.typeName();
    nodes.push_back(std::move(node));

    const std::size_t count = e.childCount();
    for (std::size_t i = 0; i < count; ++i) {
        Element* c = e.childAt(i);
        if (!c) continue;
        indexElementRecursive(*c, id, static_cast<std::uint32_t>(i));
    }
}

const PropertyObject* GaugeEditor::find(Id id) const {
    auto it = idToPtr.find(id);
    return it == idToPtr.end() ? nullptr : it->second;
}

PropertyObject* GaugeEditor::find(Id id) {
    auto it = idToPtr.find(id);
    return it == idToPtr.end() ? nullptr : it->second;
}

void GaugeEditor::setFace(GaugeFace& f) {
    face = &f;
    rebuildIndex();
}

void GaugeEditor::loadFace(const std::string& json) {
    if (!face) return;

    rapidjson::Document doc;
    doc.Parse(json.c_str());
    face->load(doc);
    rebuildIndex();
}

std::string GaugeEditor::saveFace() const {
    if (!face) return "";

    rapidjson::Document doc = face->save();
    return toString(doc);
}

void GaugeEditor::rebuildIndex() {
    nextId = 1;
    idToPtr.clear();
    ptrToId.clear();
    nodes.clear();

    if (!face) return;

    const auto elementRoot = face->getRoot();
    if (elementRoot) {
        indexElementRecursive(*elementRoot, 0, 0);
    }
}

std::string GaugeEditor::listTreeJson() const {
    rapidjson::Document d;
    d.SetArray();
    auto& a = d.GetAllocator();

    for (const auto& n : nodes) {
        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("id", n.id, a);
        obj.AddMember("parentId", n.parentId, a);
        obj.AddMember("order", n.order, a);

        rapidjson::Value t;
        t.SetString(n.type.c_str(), static_cast<rapidjson::SizeType>(n.type.size()), a);
        obj.AddMember("type", t, a);

        d.PushBack(obj, a);
    }

    return toString(d);
}

std::string GaugeEditor::listElements() const {
    rapidjson::Document d;
    d.SetArray();
    auto& a = d.GetAllocator();

    for (const auto& descriptor : Element::registry()) {
        rapidjson::Value desc(rapidjson::kObjectType);
        desc.AddMember("name", rapidjson::Value(descriptor.name, a), a);
        desc.AddMember("type", rapidjson::Value(descriptor.type, a), a);
        d.PushBack(desc, a);
    }

    return toString(d);
}

std::string GaugeEditor::getPropertiesMetaJson(Id id) const {
    const auto obj = find(id);
    if (!obj) return "[]";

    rapidjson::Document d;
    d.SetArray();
    auto& a = d.GetAllocator();

    rapidjson::Value props = obj->getPropertiesMeta(a);
    d.CopyFrom(props, a);

    return toString(d);
}

std::string GaugeEditor::getPropertiesMetaJson(Id id, const std::string& path) const {
    const auto obj = find(id);
    if (!obj) return "[]";

    if (path.empty()) {
        return getPropertiesMetaJson(id);
    }

    const PropertyObject* owner = nullptr;
    const Property* prop = nullptr;
    if (!obj->resolvePath(path, owner, prop) || !owner || !prop) {
        return "[]";
    }

    rapidjson::Document d;
    d.SetObject();
    auto& a = d.GetAllocator();

    rapidjson::Value node = owner->getPropertyMeta(*prop, a);
    d.CopyFrom(node, a);

    return toString(d);
}

std::string GaugeEditor::setPropertyJson(Id id, const std::string& path, const std::string& jsonValueText) {
    auto obj = find(id);
    if (!obj) return R"({"ok":false,"error":"NotFound"})";

    PropertyObject* owner = nullptr;
    const Property* prop = nullptr;
    if (!obj->resolvePath(path, owner, prop) || !owner || !prop || !prop->set) {
        return R"({"ok":false,"error":"UnknownProperty"})";
    }

    rapidjson::Document v;
    if (v.Parse(jsonValueText.c_str()).HasParseError()) {
        return R"({"ok":false,"error":"BadJson"})";
    }

    if (!prop->set(owner, v)) {
        return R"({"ok":false,"error":"TypeMismatch"})";
    }

    return R"({"ok":true})";
}