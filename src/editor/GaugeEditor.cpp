#include <multigauge/editor/GaugeEditor.h>

#include <algorithm>

#include <multigauge/values/Value.h>

namespace {
    Element* asElement(PropertyObject* obj) {
        return dynamic_cast<Element*>(obj);
    }

    const Element* asElement(const PropertyObject* obj) {
        return dynamic_cast<const Element*>(obj);
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

    EditorResult makeMutationResult(std::uint32_t id = 0, std::uint32_t parentId = 0) {
        EditorResult result = OkObject();
        auto& data = result.data;
        auto& a = data.GetAllocator();

        if (id != 0) {
            data.AddMember("id", id, a);
        }
        if (parentId != 0) {
            data.AddMember("parentId", parentId, a);
        }

        return result;
    }

    EditorResult makeBoundsResult(std::uint32_t id, const Rect<float>& bounds) {
        EditorResult result = OkObject();
        auto& data = result.data;
        auto& a = data.GetAllocator();

        data.AddMember("id", id, a);
        data.AddMember("x", bounds.x, a);
        data.AddMember("y", bounds.y, a);
        data.AddMember("width", bounds.width, a);
        data.AddMember("height", bounds.height, a);
        return result;
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

EditorResult GaugeEditor::listTreeJson() const {
    EditorResult result = OkArray();
    auto& d = result.data;
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

    return result;
}

EditorResult GaugeEditor::listElements() const {
    EditorResult result = OkArray();
    auto& d = result.data;
    auto& a = d.GetAllocator();

    for (const auto& descriptor : Element::registry()) {
        rapidjson::Value desc(rapidjson::kObjectType);
        desc.AddMember("name", rapidjson::Value(descriptor.name, a), a);
        desc.AddMember("type", rapidjson::Value(descriptor.id, a), a);
        d.PushBack(desc, a);
    }

    return result;
}

EditorResult GaugeEditor::listValues() const {
    EditorResult result = OkArray();
    auto& d = result.data;
    auto& a = d.GetAllocator();

    for (const Value* value : Value::list()) {
        if (!value) continue;
        d.PushBack(rapidjson::Value(value->getId(), a), a);
    }

    return result;
}

EditorResult GaugeEditor::getElementJson(Id id) const {
    const Element* element = asElement(find(id));
    if (!element) return Error("NotFound");

    EditorResult result = OkObject();
    rapidjson::Value saved(rapidjson::kObjectType);
    element->saveToJson(saved, result.data.GetAllocator());
    result.data.CopyFrom(saved, result.data.GetAllocator());
    return result;
}

EditorResult GaugeEditor::addElementJson(Id parentId, const std::string& elementJsonText) {
    return insertElementJson(parentId, elementJsonText, -1);
}

EditorResult GaugeEditor::insertElementJson(Id parentId, const std::string& elementJsonText, int index) {
    if (!face) return Error("NoFace");

    Element* parent = asElement(find(parentId));
    if (!parent) return Error("ParentNotFound");

    rapidjson::Document d;
    if (d.Parse(elementJsonText.c_str()).HasParseError()) {
        return Error("BadJson");
    }
    if (!d.IsObject()) {
        return Error("BadJson");
    }

    const std::size_t insertIndex = normalizeInsertIndex(index, parent->childCount());
    const rapidjson::Document& constDoc = d;
    Element* child = parent->insertChild(constDoc.GetObject(), insertIndex);
    if (!child) return Error("InsertFailed");

    rebuildIndex();

    const Id newId = lookupId(ptrToId, child);
    const Id resolvedParentId = lookupId(ptrToId, parent);
    return makeMutationResult(newId, resolvedParentId);
}

EditorResult GaugeEditor::moveElement(Id id, Id newParentId, int index) {
    if (!face) return Error("NoFace");

    Element* element = asElement(find(id));
    if (!element) return Error("NotFound");
    if (element->isRoot()) return Error("CannotMoveRoot");

    Element* oldParent = element->getParent();
    Element* newParent = asElement(find(newParentId));
    if (!oldParent || !newParent) return Error("ParentNotFound");
    if (element == newParent) return Error("InvalidParent");
    if (isDescendantOf(newParent, element)) return Error("InvalidParent");

    const EditorNode* node = findNode(id);
    const std::size_t oldIndex = node ? node->order : oldParent->childCount();

    std::size_t insertIndex = normalizeInsertIndex(index, newParent->childCount());
    if (oldParent == newParent && insertIndex > oldIndex) {
        --insertIndex;
    }

    OwnedElement detached = oldParent->detachChild(element);
    if (!detached) return Error("MoveFailed");

    Element* moved = detached.get();
    if (!newParent->insertChild(std::move(detached), insertIndex)) return Error("MoveFailed");

    rebuildIndex();

    const Id movedId = lookupId(ptrToId, moved);
    const Id resolvedParentId = lookupId(ptrToId, newParent);
    return makeMutationResult(movedId, resolvedParentId);
}

EditorResult GaugeEditor::removeElement(Id id) {
    if (!face) return Error("NoFace");

    Element* element = asElement(find(id));
    if (!element) return Error("NotFound");
    if (element->isRoot()) return Error("CannotRemoveRoot");

    Element* parent = element->getParent();
    if (!parent) return Error("ParentNotFound");

    const Id parentId = lookupId(ptrToId, parent);
    if (!parent->removeChild(element)) return Error("RemoveFailed");

    rebuildIndex();
    return makeMutationResult(0, parentId);
}

EditorResult GaugeEditor::getElementBoundsJson(Id id) const {
    const Element* element = asElement(find(id));
    if (!element) return Error("NotFound");

    return makeBoundsResult(id, element->getBounds());
}

EditorResult GaugeEditor::listElementBoundsJson() const {
    EditorResult result = OkArray();
    auto& d = result.data;
    auto& a = d.GetAllocator();

    for (const auto& node : nodes) {
        const Element* element = asElement(find(node.id));
        if (!element) continue;

        const Rect<float>& bounds = element->getBounds();

        rapidjson::Value obj(rapidjson::kObjectType);
        obj.AddMember("id", node.id, a);
        obj.AddMember("parentId", node.parentId, a);
        obj.AddMember("order", node.order, a);
        obj.AddMember("type", rapidjson::Value(node.type.c_str(), a), a);
        obj.AddMember("x", bounds.x, a);
        obj.AddMember("y", bounds.y, a);
        obj.AddMember("width", bounds.width, a);
        obj.AddMember("height", bounds.height, a);
        d.PushBack(std::move(obj), a);
    }

    return result;
}

EditorResult GaugeEditor::getPropertiesMetaJson(Id id) const {
    const auto obj = find(id);
    if (!obj) return Error("NotFound");

    EditorResult result = OkArray();
    rapidjson::Value props = obj->getPropertiesMeta(result.data.GetAllocator());
    result.data.CopyFrom(props, result.data.GetAllocator());
    return result;
}

EditorResult GaugeEditor::getPropertiesMetaJson(Id id, const std::string& path) const {
    const auto obj = find(id);
    if (!obj) return Error("NotFound");

    if (path.empty()) {
        return getPropertiesMetaJson(id);
    }

    const PropertyObject* owner = nullptr;
    const Property* prop = nullptr;
    if (!obj->resolvePath(path, owner, prop) || !owner || !prop) {
        return Error("UnknownProperty");
    }
    if (!prop->meta.inspectorVisible) {
        return Error("PropertyHidden");
    }

    EditorResult result = OkObject();
    rapidjson::Value node = owner->getPropertyMeta(*prop, result.data.GetAllocator());
    result.data.CopyFrom(node, result.data.GetAllocator());
    return result;
}

EditorResult GaugeEditor::setPropertyJson(Id id, const std::string& path, const std::string& jsonValueText) {
    auto obj = find(id);
    if (!obj) return Error("NotFound");

    PropertyObject* owner = nullptr;
    const Property* prop = nullptr;
    if (!obj->resolvePath(path, owner, prop) || !owner || !prop || !prop->set) {
        return Error("UnknownProperty");
    }

    rapidjson::Document v;
    if (v.Parse(jsonValueText.c_str()).HasParseError()) {
        return Error("BadJson");
    }

    if (!prop->set(owner, v)) {
        return Error("TypeMismatch");
    }

    return OkObject();
}
