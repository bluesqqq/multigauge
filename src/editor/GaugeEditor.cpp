#include <multigauge/editor/GaugeEditor.h>

#include <algorithm>

#include <multigauge/values/Value.h>

namespace {
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

    std::uint32_t lookupId(const std::unordered_map<const Element*, std::uint32_t>& ptrToId, const Element* obj) {
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

    void collectHitIds(
        const Element* element,
        const std::unordered_map<const Element*, std::uint32_t>& ptrToId,
        const Point<float>& point,
        std::vector<std::uint32_t>& hitIds) {
        if (!element) return;

        const auto it = ptrToId.find(element);
        if (it != ptrToId.end() && element->getBounds().contains(point)) {
            hitIds.push_back(it->second);
        }

        for (std::size_t i = 0; i < element->childCount(); ++i) {
            collectHitIds(element->childAt(i), ptrToId, point, hitIds);
        }
    }
}

std::string GaugeEditor::toString(const rapidjson::Value& v) {
    rapidjson::StringBuffer sb;
    rapidjson::Writer<rapidjson::StringBuffer> w(sb);
    v.Accept(w);
    return std::string(sb.GetString(), sb.GetSize());
}

void GaugeEditor::clearHistory() {
    undoHistory.clear();
    redoHistory.clear();
}

void GaugeEditor::pushHistorySnapshot(std::vector<std::string>& stack, const std::string& snapshot) {
    if (snapshot.empty()) return;
    stack.push_back(snapshot);
    if (stack.size() > kMaxHistoryEntries) {
        stack.erase(stack.begin());
    }
}

void GaugeEditor::commitMutationSnapshot(const std::string& previousJson) {
    if (!face || previousJson.empty()) return;

    const std::string currentJson = saveFace();
    if (currentJson == previousJson) return;

    pushHistorySnapshot(undoHistory, previousJson);
    redoHistory.clear();
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

const Element* GaugeEditor::find(Id id) const {
    auto it = idToPtr.find(id);
    return it == idToPtr.end() ? nullptr : it->second;
}

Element* GaugeEditor::find(Id id) {
    auto it = idToPtr.find(id);
    return it == idToPtr.end() ? nullptr : it->second;
}

void GaugeEditor::setFace(GaugeFace& f) {
    face = &f;
    clearHistory();
    rebuildIndex();
}

void GaugeEditor::loadFace(const std::string& json) {
    loadFaceInternal(json, true);
}

void GaugeEditor::loadFaceInternal(const std::string& json, bool resetHistory) {
    if (!face) return;

    rapidjson::Document doc;
    doc.Parse(json.c_str());
    face->load(doc);
    if (resetHistory) {
        clearHistory();
    }
    rebuildIndex();
}

std::string GaugeEditor::saveFace() const {
    if (!face) return "";

    rapidjson::Document doc = face->save();
    return toString(doc);
}

void GaugeEditor::syncAfterExternalLoad() {
    rebuildIndex();
}

EditorResult GaugeEditor::getHistoryState() const {
    EditorResult result = OkObject();
    auto& data = result.data;
    auto& allocator = data.GetAllocator();

    data.AddMember("canUndo", !undoHistory.empty(), allocator);
    data.AddMember("canRedo", !redoHistory.empty(), allocator);
    data.AddMember("undoDepth", static_cast<std::uint32_t>(undoHistory.size()), allocator);
    data.AddMember("redoDepth", static_cast<std::uint32_t>(redoHistory.size()), allocator);

    return result;
}

EditorResult GaugeEditor::undo() {
    if (undoHistory.empty()) return Error("NothingToUndo");

    const std::string currentJson = saveFace();
    const std::string snapshot = undoHistory.back();
    undoHistory.pop_back();
    pushHistorySnapshot(redoHistory, currentJson);
    loadFaceInternal(snapshot, false);

    return getHistoryState();
}

EditorResult GaugeEditor::redo() {
    if (redoHistory.empty()) return Error("NothingToRedo");

    const std::string currentJson = saveFace();
    const std::string snapshot = redoHistory.back();
    redoHistory.pop_back();
    pushHistorySnapshot(undoHistory, currentJson);
    loadFaceInternal(snapshot, false);

    return getHistoryState();
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
    const Element* element = find(id);
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

    Element* parent = find(parentId);
    if (!parent) return Error("ParentNotFound");

    rapidjson::Document d;
    if (d.Parse(elementJsonText.c_str()).HasParseError()) {
        return Error("BadJson");
    }
    if (!d.IsObject()) {
        return Error("BadJson");
    }

    const std::string beforeJson = saveFace();
    const std::size_t insertIndex = normalizeInsertIndex(index, parent->childCount());
    const rapidjson::Document& constDoc = d;
    Element* child = parent->insertChild(constDoc.GetObject(), insertIndex);
    if (!child) return Error("InsertFailed");

    rebuildIndex();
    commitMutationSnapshot(beforeJson);

    const Id newId = lookupId(ptrToId, child);
    const Id resolvedParentId = lookupId(ptrToId, parent);
    return makeMutationResult(newId, resolvedParentId);
}

EditorResult GaugeEditor::moveElement(Id id, Id newParentId, int index) {
    if (!face) return Error("NoFace");

    Element* element = find(id);
    if (!element) return Error("NotFound");
    if (element->isRoot()) return Error("CannotMoveRoot");

    Element* oldParent = element->getParent();
    Element* newParent = find(newParentId);
    if (!oldParent || !newParent) return Error("ParentNotFound");
    if (element == newParent) return Error("InvalidParent");
    if (isDescendantOf(newParent, element)) return Error("InvalidParent");

    const EditorNode* node = findNode(id);
    const std::size_t oldIndex = node ? node->order : oldParent->childCount();

    std::size_t insertIndex = normalizeInsertIndex(index, newParent->childCount());
    if (oldParent == newParent && insertIndex > oldIndex) {
        --insertIndex;
    }

    const std::string beforeJson = saveFace();
    OwnedElement detached = oldParent->detachChild(element);
    if (!detached) return Error("MoveFailed");

    Element* moved = detached.get();
    if (!newParent->insertChild(std::move(detached), insertIndex)) return Error("MoveFailed");

    rebuildIndex();
    commitMutationSnapshot(beforeJson);

    const Id movedId = lookupId(ptrToId, moved);
    const Id resolvedParentId = lookupId(ptrToId, newParent);
    return makeMutationResult(movedId, resolvedParentId);
}

EditorResult GaugeEditor::removeElement(Id id) {
    if (!face) return Error("NoFace");

    Element* element = find(id);
    if (!element) return Error("NotFound");
    if (element->isRoot()) return Error("CannotRemoveRoot");

    Element* parent = element->getParent();
    if (!parent) return Error("ParentNotFound");

    const Id parentId = lookupId(ptrToId, parent);
    const std::string beforeJson = saveFace();
    if (!parent->removeChild(element)) return Error("RemoveFailed");

    rebuildIndex();
    commitMutationSnapshot(beforeJson);
    return makeMutationResult(0, parentId);
}

EditorResult GaugeEditor::replaceElementJson(Id id, const std::string& json) {
    if (!face) return Error("NoFace");

    Element* element = find(id);
    if (!element) return Error("NotFound");
    if (element->isRoot()) return Error("CannotReplaceRoot");

    Element* parent = element->getParent();
    const EditorNode* node = findNode(id);
    if (!parent || !node) return Error("ParentNotFound");

    rapidjson::Document doc;
    if (doc.Parse(json.c_str()).HasParseError() || !doc.IsObject()) {
        return Error("BadJson");
    }

    const rapidjson::Document& constDoc = doc;
    OwnedElement replacement = Element::fromJson(parent, constDoc.GetObject());
    if (!replacement) return Error("ReplaceFailed");

    Element* replacementPtr = replacement.get();
    const std::string beforeJson = saveFace();
    OwnedElement detached = parent->detachChild(element);
    if (!detached) return Error("ReplaceFailed");

    if (!parent->insertChild(std::move(replacement), node->order)) {
        parent->insertChild(std::move(detached), node->order);
        return Error("ReplaceFailed");
    }

    rebuildIndex();
    commitMutationSnapshot(beforeJson);
    return makeMutationResult(lookupId(ptrToId, replacementPtr), lookupId(ptrToId, parent));
}

EditorResult GaugeEditor::bringForward(Id id) {
    const EditorNode* node = findNode(id);
    if (!node) return Error("NotFound");
    return reorderElement(id, static_cast<int>(node->order) + 1);
}

EditorResult GaugeEditor::bringToFront(Id id) {
    Element* element = find(id);
    if (!element) return Error("NotFound");
    if (element->isRoot()) return Error("CannotReorderRoot");

    Element* parent = element->getParent();
    if (!parent) return Error("ParentNotFound");
    return reorderElement(id, static_cast<int>(parent->childCount()) - 1);
}

EditorResult GaugeEditor::sendBackward(Id id) {
    const EditorNode* node = findNode(id);
    if (!node) return Error("NotFound");
    return reorderElement(id, static_cast<int>(node->order) - 1);
}

EditorResult GaugeEditor::sendToBack(Id id) {
    return reorderElement(id, 0);
}

EditorResult GaugeEditor::reorderElement(Id id, int newIndex) {
    if (!face) return Error("NoFace");

    Element* element = find(id);
    if (!element) return Error("NotFound");
    if (element->isRoot()) return Error("CannotReorderRoot");

    Element* parent = element->getParent();
    const EditorNode* node = findNode(id);
    if (!parent || !node) return Error("ParentNotFound");

    const std::size_t childCount = parent->childCount();
    if (childCount == 0) return Error("ParentNotFound");

    std::size_t targetIndex = 0;
    if (newIndex < 0) {
        targetIndex = childCount - 1;
    } else {
        targetIndex = std::min<std::size_t>(static_cast<std::size_t>(newIndex), childCount - 1);
    }

    if (targetIndex == node->order) {
        return makeMutationResult(id, lookupId(ptrToId, parent));
    }

    const std::string beforeJson = saveFace();
    OwnedElement detached = parent->detachChild(element);
    if (!detached) return Error("ReorderFailed");

    Element* moved = detached.get();
    if (!parent->insertChild(std::move(detached), targetIndex)) {
        return Error("ReorderFailed");
    }

    rebuildIndex();
    commitMutationSnapshot(beforeJson);
    return makeMutationResult(lookupId(ptrToId, moved), lookupId(ptrToId, parent));
}

EditorResult GaugeEditor::copyElement(Id id) {
    const Element* element = find(id);
    if (!element) return Error("NotFound");

    rapidjson::Document doc;
    auto& a = doc.GetAllocator();
    element->saveToJson(doc, a);
    clipboardJson = toString(doc);

    EditorResult result = OkObject();
    result.data.AddMember("id", id, result.data.GetAllocator());
    return result;
}

EditorResult GaugeEditor::pasteIntoElement(Id id) {
    if (clipboardJson.empty()) return Error("ClipboardEmpty");
    return insertElementJson(id, clipboardJson, -1);
}

EditorResult GaugeEditor::pasteToReplaceElement(Id id) {
    if (clipboardJson.empty()) return Error("ClipboardEmpty");
    return replaceElementJson(id, clipboardJson);
}

EditorResult GaugeEditor::duplicateElement(Id id) {
    Element* element = find(id);
    if (!element) return Error("NotFound");
    if (element->isRoot()) return Error("CannotDuplicateRoot");

    const EditorNode* node = findNode(id);
    Element* parent = element->getParent();
    if (!node || !parent) return Error("ParentNotFound");

    rapidjson::Document doc;
    auto& a = doc.GetAllocator();
    element->saveToJson(doc, a);
    return insertElementJson(lookupId(ptrToId, parent), toString(doc), static_cast<int>(node->order) + 1);
}

EditorResult GaugeEditor::getElementBoundsJson(Id id) const {
    const Element* element = find(id);
    if (!element) return Error("NotFound");

    return makeBoundsResult(id, element->getBounds());
}

EditorResult GaugeEditor::listElementBoundsJson() const {
    EditorResult result = OkArray();
    auto& d = result.data;
    auto& a = d.GetAllocator();

    for (const auto& node : nodes) {
        const Element* element = find(node.id);
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

GaugeEditor::Id GaugeEditor::hitTest(float x, float y, int index) const {
    if (index < 0) return 0;

    const EditorResult hits = hitTestAll(x, y);
    if (!hits.ok || !hits.data.IsArray()) return 0;

    const auto hitIndex = static_cast<rapidjson::SizeType>(index);
    if (hitIndex >= hits.data.Size() || !hits.data[hitIndex].IsUint()) return 0;
    return hits.data[hitIndex].GetUint();
}

EditorResult GaugeEditor::hitTestAll(float x, float y) const {
    if (!face) return Error("NoFace");

    EditorResult result = OkArray();
    std::vector<std::uint32_t> hitIds;
    collectHitIds(face->getRoot(), ptrToId, Point<float>(x, y), hitIds);

    std::reverse(hitIds.begin(), hitIds.end());

    auto& data = result.data;
    auto& a = data.GetAllocator();
    data.Reserve(static_cast<rapidjson::SizeType>(hitIds.size()), a);
    for (const std::uint32_t id : hitIds) {
        data.PushBack(id, a);
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

    const std::string beforeJson = saveFace();
    if (!prop->set(owner, v)) {
        return Error("TypeMismatch");
    }

    commitMutationSnapshot(beforeJson);
    return OkObject();
}
