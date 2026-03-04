#include <multigauge/editor/GaugeEditor.h>

std::string GaugeEditor::toString(const rapidjson::Value &v) {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);
    v.Accept(w);
    return std::string(sb.GetString(), sb.GetSize());
}

void GaugeEditor::indexEditable(Editable &e, Id parentId, std::uint32_t order, const std::string &typeName) {
    const Id id = nextId++;
    idToPtr[id] = &e;
    ptrToId[&e] = id;

    EditorNode node;
    node.id = id;
    node.parentId = parentId;
    node.order = order;
    node.type = typeName;
    nodes.push_back(std::move(node));
}

void GaugeEditor::indexElementRecursive(Element &e, Id parentId, std::uint32_t order) {
    const Id id = nextId++;
    idToPtr[id] = &e;
    ptrToId[&e] = id;

    EditorNode node;
    node.id = id;
    node.parentId = parentId;
    node.order = order;
    node.type = e.editorTypeName();
    nodes.push_back(std::move(node));

    const std::size_t count = e.childCount();
    for (std::size_t i = 0; i < count; ++i) {
        Element* c = e.childAt(i);
        if (!c) continue;
        indexElementRecursive(*c, id, static_cast<std::uint32_t>(i));
    }
}

const Editable *GaugeEditor::find(Id id) const {
    auto it = idToPtr.find(id);
    return it == idToPtr.end() ? nullptr : it->second;
}

Editable *GaugeEditor::find(Id id) {
    auto it = idToPtr.find(id);
    return it == idToPtr.end() ? nullptr : it->second;
}

rapidjson::Value GaugeEditor::exportEditableProps(const Editable &e, rapidjson::Document::AllocatorType &a) const {
    rapidjson::Value props(rapidjson::kObjectType);
    e.saveProperties(props, a);
    return props;
}

rapidjson::Value GaugeEditor::exportElementRecursive(const Element &e, rapidjson::Document::AllocatorType &a) const {
    rapidjson::Value obj(rapidjson::kObjectType);

    // type
    {
        const std::string tname = e.editorTypeName();
        rapidjson::Value t;
        t.SetString(tname.c_str(), static_cast<rapidjson::SizeType>(tname.size()), a);
        obj.AddMember("type", t, a);
    }

    // props
    {
        rapidjson::Value props = exportEditableProps(e, a);
        obj.AddMember("props", props, a);
    }

    // children
    {
        rapidjson::Value arr(rapidjson::kArrayType);
        const std::size_t count = e.childCount();
        for (std::size_t i = 0; i < count; ++i) {
            const Element* c = e.childAt(i);
            if (!c) continue;
            arr.PushBack(exportElementRecursive(*c, a), a);
        }
        obj.AddMember("children", arr, a);
    }

    return obj;
}

void GaugeEditor::setFace(GaugeFace &f) {
    face = &f;
    rebuildIndex();
}

void GaugeEditor::rebuildIndex() {
    nextId = 1;
    faceId = 0;
    idToPtr.clear();
    ptrToId.clear();
    nodes.clear();

    if (!face) return;

    faceId = nextId;
    indexEditable(*face, 0, 0, "GaugeFace");

    const auto elementRoot = face->getRoot();
    if (elementRoot) indexElementRecursive(*elementRoot, faceId, 0);
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

std::string GaugeEditor::savePropertiesJson(Id id) const {
    const auto e = find(id);
    if (!e) return R"({"ok":false,"error":"NotFound"})";

    rapidjson::Document d;
    d.SetObject();
    auto& a = d.GetAllocator();

    rapidjson::Value props;
    e->saveProperties(props, a);

    rapidjson::Value meta(rapidjson::kArrayType);
    {
        auto pl = e->propertyList();
        for (std::size_t i = 0; i < pl.count; ++i) {
            const auto& p = pl.props[i];
            rapidjson::Value m(rapidjson::kObjectType);

            m.AddMember("key", rapidjson::Value(p.key, a), a);
            m.AddMember("name", rapidjson::Value(p.name ? p.name : p.key, a), a);
            m.AddMember("description", rapidjson::Value(p.description ? p.description : "", a), a);

            meta.PushBack(m, a);
        }
    }

    d.AddMember("ok", true, a);
    d.AddMember("properties", props, a);
    d.AddMember("meta", meta, a);
    return toString(d);
}

std::string GaugeEditor::loadPropertyJson(Id id, const std::string &propName, const std::string &jsonValueText) {
    auto e = find(id);
    if (!e) return R"({"ok":false,"error":"NotFound"})";

    const Property* p = e->findProperty(propName.c_str());
    if (!p || !p->set) return R"({"ok":false,"error":"UnknownProperty"})";

    rapidjson::Document v;
    if (v.Parse(jsonValueText.c_str()).HasParseError()) return R"({"ok":false,"error":"BadJson"})";

    if (!p->set(e, v)) return R"({"ok":false,"error":"TypeMismatch"})";

    return R"({"ok":true})";
}

std::string GaugeEditor::patchPropertyJson(Id id, const std::string &propName, const std::string &patchObjectText) {
    auto e = find(id);
    if (!e) return R"({"ok":false,"error":"NotFound"})";

    const Property *p = e->findProperty(propName.c_str());
    if (!p || !p->get || !p->set) return R"({"ok":false,"error":"UnknownProperty"})";

    rapidjson::Document d;
    d.SetObject();
    auto &a = d.GetAllocator();

    rapidjson::Value current;
    if (!p->get(e, current, a)) return R"({"ok":false,"error":"GetFailed"})";
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

    if (!p->set(e, current)) return R"({"ok":false,"error":"TypeMismatch"})";

    return R"({"ok":true})";
}

std::string GaugeEditor::exportFaceJson() const {
    if (!face) return R"({"ok":false,"error":"NoFace"})";

    rapidjson::Document d;
    d.SetObject();
    auto& a = d.GetAllocator();

    d.AddMember("ok", true, a);

    // face object: props + root element
    rapidjson::Value faceObj(rapidjson::kObjectType);

    // face props (via Editable)
    {
        rapidjson::Value props = exportEditableProps(*face, a);
        faceObj.AddMember("props", props, a);
    }

    // root element
    {
        const Element* root = face->getRoot();
        if (root) {
            faceObj.AddMember("root", exportElementRecursive(*root, a), a);
        } else {
            faceObj.AddMember("root", rapidjson::Value(rapidjson::kNullType), a);
        }
    }

    d.AddMember("face", faceObj, a);
    return toString(d);
}