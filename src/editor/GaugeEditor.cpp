#include <multigauge/editor/GaugeEditor.h>

#include <algorithm>

#include <multigauge/values/Value.h>

namespace {
    std::string makeResult(bool ok, const char* error = nullptr, std::uint32_t id = 0, std::uint32_t parentId = 0) {
        rapidjson::Document d;
        d.SetObject();
        auto& a = d.GetAllocator();

        d.AddMember("ok", ok, a);
        if (error) {
            d.AddMember("error", rapidjson::Value(error, a), a);
        }
        if (id != 0) {
            d.AddMember("id", id, a);
        }
        if (parentId != 0) {
            d.AddMember("parentId", parentId, a);
        }

        rapidjson::StringBuffer sb;
        rapidjson::Writer<rapidjson::StringBuffer> w(sb);
        d.Accept(w);
        return std::string(sb.GetString(), sb.GetSize());
    }

    Element* asElement(PropertyObject* obj) {
        return dynamic_cast<Element*>(obj);
    }

    std::size_t normalizeInsertIndex(int index, std::size_t count) {
        if (index < 0) return count;
        return std::min<std::size_t>(static_cast<std::size_t>(index), count);
    }

    bool isDescendantOf(const Element* node, const Element* ancestor) {
        for (auto* current = node; current; current = current->getParent()) {
            if (current == ancestor) return true;
        }
        return false;
    }

    std::uint32_t lookupId(const std::unordered_map<PropertyObject*, std::uint32_t>& ptrToId, PropertyObject* obj) {
        auto it = ptrToId.find(obj);
        return it == ptrToId.end() ? 0u : it->second;
    }
}

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

const EditorNode* GaugeEditor::findNode(Id id) const {
    for (const auto& node : nodes) {
        if (node.id == id) return &node;
    }
    return nullptr;
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
        desc.AddMember("type", rapidjson::Value(descriptor.id, a), a);
        d.PushBack(desc, a);
    }

    return toString(d);
}

std::string GaugeEditor::listValues() const {
    rapidjson::Document d;
    d.SetArray();
    auto& a = d.GetAllocator();

    for (const Value* value : Value::list()) {
        if (!value) continue;
        d.PushBack(rapidjson::Value(value->getId(), a), a);
    }

    return toString(d);
}

std::string GaugeEditor::addElement(Id parentId, const std::string& type) {
    return insertElement(parentId, type, -1);
}

std::string GaugeEditor::insertElement(Id parentId, const std::string& type, int index) {
    if (!face) return makeResult(false, "NoFace");

    Element* parent = asElement(find(parentId));
    if (!parent) return makeResult(false, "ParentNotFound");

    const auto* descriptor = Element::registry().find(type.c_str());
    if (!descriptor) return makeResult(false, "UnknownType");

    rapidjson::Document d;
    d.SetObject();
    auto& a = d.GetAllocator();
    d.AddMember(rapidjson::Value(TYPE_KEY, a), rapidjson::Value(descriptor->id, a), a);

    const std::size_t insertIndex = normalizeInsertIndex(index, parent->childCount());
    const rapidjson::Document& constDoc = d;
    Element* child = parent->insertChild(constDoc.GetObject(), insertIndex);
    if (!child) return makeResult(false, "InsertFailed");

    rebuildIndex();

    const Id newId = lookupId(ptrToId, child);
    const Id resolvedParentId = lookupId(ptrToId, parent);
    return makeResult(true, nullptr, newId, resolvedParentId);
}

std::string GaugeEditor::moveElement(Id id, Id newParentId, int index) {
    if (!face) return makeResult(false, "NoFace");

    Element* element = asElement(find(id));
    if (!element) return makeResult(false, "NotFound");
    if (element->isRoot()) return makeResult(false, "CannotMoveRoot");

    Element* oldParent = element->getParent();
    Element* newParent = asElement(find(newParentId));
    if (!oldParent || !newParent) return makeResult(false, "ParentNotFound");
    if (element == newParent) return makeResult(false, "InvalidParent");
    if (isDescendantOf(newParent, element)) return makeResult(false, "InvalidParent");

    const EditorNode* node = findNode(id);
    const std::size_t oldIndex = node ? node->order : oldParent->childCount();

    std::size_t insertIndex = normalizeInsertIndex(index, newParent->childCount());
    if (oldParent == newParent && insertIndex > oldIndex) {
        --insertIndex;
    }

    OwnedElement detached = oldParent->detachChild(element);
    if (!detached) return makeResult(false, "MoveFailed");

    Element* moved = detached.get();
    if (!newParent->insertChild(std::move(detached), insertIndex)) return makeResult(false, "MoveFailed");

    rebuildIndex();

    const Id movedId = lookupId(ptrToId, moved);
    const Id resolvedParentId = lookupId(ptrToId, newParent);
    return makeResult(true, nullptr, movedId, resolvedParentId);
}

std::string GaugeEditor::removeElement(Id id) {
    if (!face) return makeResult(false, "NoFace");

    Element* element = asElement(find(id));
    if (!element) return makeResult(false, "NotFound");
    if (element->isRoot()) return makeResult(false, "CannotRemoveRoot");

    Element* parent = element->getParent();
    if (!parent) return makeResult(false, "ParentNotFound");

    const Id parentId = lookupId(ptrToId, parent);
    if (!parent->removeChild(element)) return makeResult(false, "RemoveFailed");

    rebuildIndex();
    return makeResult(true, nullptr, 0, parentId);
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

std::string GaugeEditor::listValues() {
    return std::string();
}
