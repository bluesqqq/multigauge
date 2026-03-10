#include <multigauge/editor/GaugeEditor.h>

std::string GaugeEditor::toString(const rapidjson::Value &v) {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);
    v.Accept(w);
    return std::string(sb.GetString(), sb.GetSize());
}

void GaugeEditor::indexElementRecursive(Element &e, Id parentId, std::uint32_t order) {
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

const PropertyObject *GaugeEditor::find(Id id) const {
    auto it = idToPtr.find(id);
    return it == idToPtr.end() ? nullptr : it->second;
}

PropertyObject *GaugeEditor::find(Id id) {
    auto it = idToPtr.find(id);
    return it == idToPtr.end() ? nullptr : it->second;
}

void GaugeEditor::setFace(GaugeFace &f) {
    face = &f;
    rebuildIndex();
}

void GaugeEditor::loadFace(const std::string &json) {
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
    if (elementRoot) indexElementRecursive(*elementRoot, 0, 0);
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

std::string GaugeEditor::getPropertiesMetaJson(Id id) const {
    const auto obj = find(id);
    if (!obj) return R"({"ok":false,"error":"NotFound"})";

    return toString(obj->getPropertiesMeta(rapidjson::Document().GetAllocator()));
}

std::string GaugeEditor::loadPropertyJson(Id id, const std::string &propName, const std::string &jsonValueText) {
    auto obj = find(id);
    if (!obj) return R"({"ok":false,"error":"NotFound"})";

    const Property* property = obj->findProperty(propName.c_str());
    if (!property || !property->set) return R"({"ok":false,"error":"UnknownProperty"})";

    rapidjson::Document v;
    if (v.Parse(jsonValueText.c_str()).HasParseError()) return R"({"ok":false,"error":"BadJson"})";

    if (!property->set(obj, v)) return R"({"ok":false,"error":"TypeMismatch"})";

    return R"({"ok":true})";
}

std::string GaugeEditor::patchPropertyJson(Id id, const std::string &propName, const std::string &patchObjectText) {
    auto obj = find(id);
    if (!obj) return R"({"ok":false,"error":"NotFound"})";

    const Property *property = obj->findProperty(propName.c_str());
    if (!property || !property->get || !property->set) return R"({"ok":false,"error":"UnknownProperty"})";

    rapidjson::Document d;
    d.SetObject();
    auto &a = d.GetAllocator();

    rapidjson::Value current;
    if (!property->get(obj, current, a)) return R"({"ok":false,"error":"GetFailed"})";
    if (!current.IsObject()) return R"({"ok":false,"error":"NotObject"})";

    rapidjson::Document patch;
    if (patch.Parse(patchObjectText.c_str()).HasParseError()) return R"({"ok":false,"error":"BadJson"})";
    if (!patch.IsObject()) return R"({"ok":false,"error":"PatchNotObject"})";

    for (auto it = patch.MemberBegin(); it != patch.MemberEnd(); ++it) {
        rapidjson::Value name;
        name.SetString(it->name.GetString(), it->name.GetStringLength(), a);

        rapidjson::Value val;
        val.CopyFrom(it->value, a);

        auto existing = current.FindMember(it->name);
        if (existing != current.MemberEnd()) existing->value = std::move(val);
        else current.AddMember(name, val, a);
    }

    if (!property->set(obj, current)) return R"({"ok":false,"error":"TypeMismatch"})";

    return R"({"ok":true})";
}
