#include <multigauge/editor/Editor.h>

#include <algorithm>
#include <memory>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <multigauge/values/Value.h>

namespace {

struct FaceInsertState {
    GaugeFace* face = nullptr;
    std::size_t index = 0;
    Editor::Id faceId = 0;
    std::vector<std::vector<Editor::Id>> rootIds;
    bool initialized = false;
};

struct FaceRemoveState {
    GaugeFace* face = nullptr;
    Editor::Id faceId = 0;
    std::size_t index = 0;
    std::vector<std::vector<Editor::Id>> rootIds;
};

struct FaceReorderState {
    GaugeFace* face = nullptr;
    Editor::Id faceId = 0;
    std::size_t from = 0;
    std::size_t to = 0;
};

struct ElementSnapshot {
    std::string json;
    std::vector<Editor::Id> ids;
};

struct ElementCreateState {
    Editor::Id parentId = 0;
    std::size_t index = 0;
    std::string json;
    ElementSnapshot created;
    bool initialized = false;
};

struct ElementRemoveState {
    Editor::Id parentId = 0;
    std::size_t index = 0;
    ElementSnapshot removed;
};

struct ElementReorderState {
    Editor::Id parentId = 0;
    Editor::Id elementId = 0;
    std::size_t from = 0;
    std::size_t to = 0;
};

struct ElementMoveState {
    Editor::Id elementId = 0;
    Editor::Id oldParentId = 0;
    Editor::Id newParentId = 0;
    std::size_t fromIndex = 0;
    std::size_t toIndex = 0;
};

struct ReplaceState {
    Editor::Id targetId = 0;
    Editor::Id faceId = 0;
    Editor::Id parentId = 0;
    std::size_t index = 0;
    ElementSnapshot before;
    ElementSnapshot after;
    std::string pendingJson;
    bool initialized = false;
};

struct PropertySetState {
    Editor::Id nodeId = 0;
    std::string path;
    std::string beforeJson;
    std::string afterJson;
};

rapidjson::Document parseJson(const std::string& json) {
    rapidjson::Document doc;
    doc.Parse(json.c_str());
    return doc;
}

std::string toString(const rapidjson::Value& value) {
    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    value.Accept(writer);
    return buffer.GetString();
}

std::string serializeElement(const Element& element) {
    rapidjson::Document doc;
    doc.SetObject();
    element.saveToJson(doc, doc.GetAllocator());
    return toString(doc);
}

void collectSubtreeIds(const Editor& editor, const Element* element, std::vector<Editor::Id>& ids) {
    if (!element) return;

    ids.push_back(editor.idOf(element));
    for (std::size_t i = 0; i < element->childCount(); ++i) {
        collectSubtreeIds(editor, element->childAt(i), ids);
    }
}

ElementSnapshot snapshotElement(const Editor& editor, const Element& element) {
    ElementSnapshot snapshot;
    snapshot.json = serializeElement(element);
    collectSubtreeIds(editor, &element, snapshot.ids);
    return snapshot;
}

std::vector<std::vector<Editor::Id>> snapshotFaceRootIds(const Editor& editor, const GaugeFace& face) {
    std::vector<std::vector<Editor::Id>> rootIds;
    rootIds.reserve(face.childCount());

    for (std::size_t i = 0; i < face.childCount(); ++i) {
        std::vector<Editor::Id> ids;
        collectSubtreeIds(editor, face.childAt(i), ids);
        rootIds.push_back(std::move(ids));
    }

    return rootIds;
}

void addString(rapidjson::Value& object,
               const char* key,
               const char* value,
               rapidjson::Document::AllocatorType& allocator) {
    object.AddMember(rapidjson::Value(key, allocator),
                     rapidjson::Value(value, allocator),
                     allocator);
}

bool elementContains(const Element* root, const Element* target) {
    if (!root || !target) return false;
    if (root == target) return true;

    for (std::size_t i = 0; i < root->childCount(); ++i) {
        if (elementContains(root->childAt(i), target)) return true;
    }

    return false;
}

std::size_t indexOfRoot(const GaugeFace& face, const Element* element) {
    for (std::size_t i = 0; i < face.childCount(); ++i) {
        if (face.childAt(i) == element) return i;
    }

    return Editor::Append;
}

std::size_t indexOfChild(const Element& parent, const Element* child) {
    for (std::size_t i = 0; i < parent.childCount(); ++i) {
        if (parent.childAt(i) == child) return i;
    }

    return Editor::Append;
}

GaugeFace* findOwningFace(const std::vector<GaugeFace*>& faces, const Element* root) {
    if (!root || root->getParent()) return nullptr;

    for (GaugeFace* face : faces) {
        if (!face) continue;
        if (indexOfRoot(*face, root) != Editor::Append) return face;
    }

    return nullptr;
}

EditorResult okWithId(Editor::Id id) {
    EditorResult result = OkObject();
    auto& allocator = result.data.GetAllocator();
    result.data.AddMember("id", id, allocator);
    return result;
}

EditorResult okWithIds(Editor::Id id, Editor::Id parentId) {
    EditorResult result = OkObject();
    auto& allocator = result.data.GetAllocator();
    result.data.AddMember("id", id, allocator);
    result.data.AddMember("parentId", parentId, allocator);
    return result;
}

void appendElementHierarchyNode(const Editor& editor,
                                const Element& element,
                                rapidjson::Value& nodes,
                                rapidjson::Document::AllocatorType& allocator) {
    const Editor::Id id = editor.idOf(&element);
    rapidjson::Value nodeKey(std::to_string(id).c_str(), allocator);
    rapidjson::Value nodeValue(rapidjson::kObjectType);

    addString(nodeValue, "kind", "element", allocator);
    addString(nodeValue, "type", element.typeName(), allocator);
    addString(nodeValue, "name", element.typeName(), allocator);

    rapidjson::Value children(rapidjson::kArrayType);
    children.Reserve(static_cast<rapidjson::SizeType>(element.childCount()), allocator);
    for (std::size_t i = 0; i < element.childCount(); ++i) {
        const Element* child = element.childAt(i);
        if (!child) continue;
        children.PushBack(editor.idOf(child), allocator);
    }
    nodeValue.AddMember("children", std::move(children), allocator);
    nodes.AddMember(std::move(nodeKey), std::move(nodeValue), allocator);

    for (std::size_t i = 0; i < element.childCount(); ++i) {
        const Element* child = element.childAt(i);
        if (!child) continue;
        appendElementHierarchyNode(editor, *child, nodes, allocator);
    }
}

} // namespace

Editor::NodeRef* Editor::getNode(Id id) {
    auto it = nodes.find(id);
    return it == nodes.end() ? nullptr : &it->second;
}

const Editor::NodeRef* Editor::getNode(Id id) const {
    auto it = nodes.find(id);
    return it == nodes.end() ? nullptr : &it->second;
}

Editor::Id Editor::registerFace(GaugeFace* face) {
    if (!face) return 0;

    auto it = faceToId.find(face);
    if (it != faceToId.end()) return it->second;

    const Id id = makeId();
    return registerFaceWithId(id, face);
}

Editor::Id Editor::registerFaceWithId(Id id, GaugeFace* face) {
    if (!face || id == 0) return 0;
    if (nextId <= id) nextId = id + 1;

    nodes[id] = NodeRef(face);
    faceToId[face] = id;
    return id;
}

Editor::Id Editor::registerElement(Element* element) {
    if (!element) return 0;

    auto it = elementToId.find(element);
    if (it != elementToId.end()) return it->second;

    const Id id = makeId();
    return registerElementWithId(id, element);
}

Editor::Id Editor::registerElementWithId(Id id, Element* element) {
    if (!element || id == 0) return 0;
    if (nextId <= id) nextId = id + 1;

    nodes[id] = NodeRef(element);
    elementToId[element] = id;
    return id;
}

void Editor::registerSubtree(Element* element) {
    if (!element) return;

    registerElement(element);
    for (std::size_t i = 0; i < element->childCount(); ++i) {
        registerSubtree(element->childAt(i));
    }
}

bool Editor::registerSubtreeWithIds(Element* element, const std::vector<Id>& ids, std::size_t& nextIndex) {
    if (!element) return false;
    if (nextIndex >= ids.size()) return false;

    if (!registerElementWithId(ids[nextIndex++], element)) return false;

    for (std::size_t i = 0; i < element->childCount(); ++i) {
        if (!registerSubtreeWithIds(element->childAt(i), ids, nextIndex)) return false;
    }

    return true;
}

void Editor::unregisterFace(GaugeFace* face) {
    if (!face) return;

    for (std::size_t i = 0; i < face->childCount(); ++i) {
        unregisterElementRecursive(face->childAt(i));
    }

    if (auto it = faceToId.find(face); it != faceToId.end()) {
        nodes.erase(it->second);
        faceToId.erase(it);
    }
}

void Editor::unregisterElementRecursive(Element* element) {
    if (!element) return;

    for (std::size_t i = 0; i < element->childCount(); ++i) {
        unregisterElementRecursive(element->childAt(i));
    }

    if (auto it = elementToId.find(element); it != elementToId.end()) {
        nodes.erase(it->second);
        elementToId.erase(it);
    }
}

GaugeFace* Editor::getFaceById(Id id) {
    NodeRef* node = getNode(id);
    return (node && node->kind == NodeKind::Face) ? node->face : nullptr;
}

const GaugeFace* Editor::getFaceById(Id id) const {
    const NodeRef* node = getNode(id);
    return (node && node->kind == NodeKind::Face) ? node->face : nullptr;
}

Element* Editor::getElementById(Id id) {
    NodeRef* node = getNode(id);
    return (node && node->kind == NodeKind::Element) ? node->element : nullptr;
}

const Element* Editor::getElementById(Id id) const {
    const NodeRef* node = getNode(id);
    return (node && node->kind == NodeKind::Element) ? node->element : nullptr;
}

PropertyObject* Editor::getObjectById(Id id) {
    if (GaugeFace* face = getFaceById(id)) return face;
    if (Element* element = getElementById(id)) return element;
    return nullptr;
}

const PropertyObject* Editor::getObjectById(Id id) const {
    if (const GaugeFace* face = getFaceById(id)) return face;
    if (const Element* element = getElementById(id)) return element;
    return nullptr;
}

Editor::ElementContainerRef Editor::getElementContainerById(Id id) {
    if (GaugeFace* face = getFaceById(id)) return { face, nullptr };
    if (Element* element = getElementById(id)) return { nullptr, element };
    return {};
}

Editor::ElementContainerRef Editor::getElementContainerOf(Element* element) {
    if (!element) return {};
    if (Element* parent = element->getParent()) return { nullptr, parent };
    if (GaugeFace* face = findOwningFace(faces, element)) return { face, nullptr };
    return {};
}

Editor::Id Editor::idOfContainer(const ElementContainerRef& container) const {
    if (container.face) return idOf(container.face);
    if (container.element) return idOf(container.element);
    return 0;
}

std::size_t Editor::childCountOf(const ElementContainerRef& container) const {
    if (container.face) return container.face->childCount();
    if (container.element) return container.element->childCount();
    return 0;
}

bool Editor::insertIntoContainer(const ElementContainerRef& container, OwnedElement child, std::size_t index) {
    if (container.face) return static_cast<bool>(container.face->insertChild(std::move(child), index));
    if (container.element) return static_cast<bool>(container.element->insertChild(std::move(child), index));
    return false;
}

OwnedElement Editor::removeFromContainer(const ElementContainerRef& container, Element* child) {
    if (container.face) return container.face->removeChild(child);
    if (container.element) return container.element->removeChild(child);
    return {};
}

std::size_t Editor::indexInContainer(const ElementContainerRef& container, const Element* child) const {
    if (container.face) return indexOfRoot(*container.face, child);
    if (container.element) return indexOfChild(*container.element, child);
    return Append;
}

void Editor::clear() {
    faces.clear();
    ownedFaces.clear();
    nodes.clear();
    faceToId.clear();
    elementToId.clear();
    nextId = 1;
    history = History();
    clipboard.clear();
}

void Editor::loadDocument(const std::string& json) {
    clear();

    rapidjson::Document doc = parseJson(json);
    if (!doc.IsArray()) return;

    for (const auto& faceJson : doc.GetArray()) {
        if (!faceJson.IsObject()) continue;

        auto face = std::make_unique<GaugeFace>();
        face->load(faceJson);

        GaugeFace* raw = face.get();
        ownedFaces.push_back(std::move(face));

        faces.push_back(raw);
        registerFace(raw);

        for (std::size_t i = 0; i < raw->childCount(); ++i) {
            registerSubtree(raw->childAt(i));
        }
    }
}

std::string Editor::saveDocument() const {
    rapidjson::Document doc;
    doc.SetArray();
    auto& allocator = doc.GetAllocator();

    for (const GaugeFace* face : faces) {
        if (!face) continue;

        rapidjson::Document saved = face->save();
        rapidjson::Value copy;
        copy.CopyFrom(saved, allocator);
        doc.PushBack(std::move(copy), allocator);
    }

    return toString(doc);
}

bool Editor::hasNode(Id id) const {
    return getNode(id) != nullptr;
}

bool Editor::isFace(Id id) const {
    return getFaceById(id) != nullptr;
}

bool Editor::isElement(Id id) const {
    return getElementById(id) != nullptr;
}

Editor::Id Editor::idOf(const GaugeFace* face) const {
    auto it = faceToId.find(const_cast<GaugeFace*>(face));
    return it == faceToId.end() ? 0 : it->second;
}

Editor::Id Editor::idOf(const Element* element) const {
    auto it = elementToId.find(const_cast<Element*>(element));
    return it == elementToId.end() ? 0 : it->second;
}

EditorResult Editor::getHierarchy() const {
    EditorResult result = OkObject();
    auto& allocator = result.data.GetAllocator();

    rapidjson::Value roots(rapidjson::kArrayType);
    roots.Reserve(static_cast<rapidjson::SizeType>(faces.size()), allocator);

    rapidjson::Value nodesValue(rapidjson::kObjectType);
    for (const GaugeFace* face : faces) {
        if (!face) continue;

        const Id faceId = idOf(face);
        roots.PushBack(faceId, allocator);

        rapidjson::Value nodeKey(std::to_string(faceId).c_str(), allocator);
        rapidjson::Value nodeValue(rapidjson::kObjectType);
        addString(nodeValue, "kind", "face", allocator);
        addString(nodeValue, "name", face->typeName(), allocator);

        rapidjson::Value children(rapidjson::kArrayType);
        children.Reserve(static_cast<rapidjson::SizeType>(face->childCount()), allocator);
        for (std::size_t i = 0; i < face->childCount(); ++i) {
            const Element* child = face->childAt(i);
            if (!child) continue;
            children.PushBack(idOf(child), allocator);
        }
        nodeValue.AddMember("children", std::move(children), allocator);
        nodesValue.AddMember(std::move(nodeKey), std::move(nodeValue), allocator);

        for (std::size_t i = 0; i < face->childCount(); ++i) {
            const Element* child = face->childAt(i);
            if (!child) continue;
            appendElementHierarchyNode(*this, *child, nodesValue, allocator);
        }
    }

    result.data.AddMember("roots", std::move(roots), allocator);
    result.data.AddMember("nodes", std::move(nodesValue), allocator);
    return result;
}

EditorResult Editor::listElementTypes() const {
    EditorResult result = OkArray();
    auto& data = result.data;
    auto& allocator = data.GetAllocator();

    for (const auto& descriptor : Element::registry()) {
        rapidjson::Value entry(rapidjson::kObjectType);
        entry.AddMember("name", rapidjson::Value(descriptor.name ? descriptor.name : "", allocator), allocator);
        entry.AddMember("type", rapidjson::Value(descriptor.id ? descriptor.id : "", allocator), allocator);
        data.PushBack(std::move(entry), allocator);
    }

    return result;
}

EditorResult Editor::listValueIDs() const {
    EditorResult result = OkArray();
    auto& data = result.data;
    auto& allocator = data.GetAllocator();

    for (const Value* value : Value::list()) {
        if (!value) continue;
        data.PushBack(rapidjson::Value(value->getId(), allocator), allocator);
    }

    return result;
}

EditorResult Editor::insertFace(GaugeFace& face, FacePlacement where) {
    if (faceToId.count(&face)) return Error("Face is already registered");

    const std::size_t index = clampIndex(where.index, faces.size());
    auto state = std::make_shared<FaceInsertState>();
    state->face = &face;
    state->index = index;
    state->faceId = makeId();

    const bool committed = history.commit({
        "insert face",
        [this, state]() {
            faces.insert(faces.begin() + static_cast<std::ptrdiff_t>(state->index), state->face);
            registerFaceWithId(state->faceId, state->face);

            if (!state->initialized) {
                state->rootIds.clear();
                for (std::size_t i = 0; i < state->face->childCount(); ++i) {
                    registerSubtree(state->face->childAt(i));
                }
                state->rootIds = snapshotFaceRootIds(*this, *state->face);
                state->initialized = true;
                return true;
            }
            
            if (state->rootIds.size() != state->face->childCount()) return false;
            for (std::size_t i = 0; i < state->face->childCount(); ++i) {
                std::size_t next = 0;
                if (!registerSubtreeWithIds(state->face->childAt(i), state->rootIds[i], next)) return false;
                if (next != state->rootIds[i].size()) return false;
            }
            return true;
        },
        [this, state]() {
            faces.erase(std::remove(faces.begin(), faces.end(), state->face), faces.end());
            unregisterFace(state->face);
            return true;
        }
    });

    return committed ? okWithId(state->faceId) : Error("Failed to insert face");
}

EditorResult Editor::removeFace(Id faceId) {
    GaugeFace* face = getFaceById(faceId);
    if (!face) return Error("Invalid face id");

    const auto it = std::find(faces.begin(), faces.end(), face);
    if (it == faces.end()) return Error("Face is not in the face list");

    const std::size_t index = static_cast<std::size_t>(std::distance(faces.begin(), it));
    auto state = std::make_shared<FaceRemoveState>();
    state->face = face;
    state->faceId = faceId;
    state->index = index;
    state->rootIds = snapshotFaceRootIds(*this, *face);

    const bool committed = history.commit({
        "remove face",
        [this, state]() {
            GaugeFace* current = state->face;
            if (!current) return false;

            faces.erase(std::remove(faces.begin(), faces.end(), current), faces.end());
            unregisterFace(current);
            return true;
        },
        [this, state]() {
            GaugeFace* faceToRestore = state->face;
            if (!faceToRestore) return false;

            faces.insert(faces.begin() + static_cast<std::ptrdiff_t>(state->index), faceToRestore);
            registerFaceWithId(state->faceId, faceToRestore);
            if (state->rootIds.size() != faceToRestore->childCount()) return false;
            for (std::size_t i = 0; i < faceToRestore->childCount(); ++i) {
                std::size_t next = 0;
                if (!registerSubtreeWithIds(faceToRestore->childAt(i), state->rootIds[i], next)) return false;
                if (next != state->rootIds[i].size()) return false;
            }
            return true;
        }
    });

    return committed ? OkObject() : Error("Failed to remove face");
}

EditorResult Editor::reorderFace(Id faceId, std::size_t index) {
    GaugeFace* face = getFaceById(faceId);
    if (!face) return Error("Invalid face id");

    const auto it = std::find(faces.begin(), faces.end(), face);
    if (it == faces.end()) return Error("Face is not in the face list");

    const std::size_t oldIndex = static_cast<std::size_t>(std::distance(faces.begin(), it));
    const std::size_t newIndex = clampIndex(index, faces.size() - 1);
    if (oldIndex == newIndex) return OkObject();

    auto state = std::make_shared<FaceReorderState>();
    state->face = face;
    state->faceId = faceId;
    state->from = oldIndex;
    state->to = newIndex;

    const bool committed = history.commit({
        "reorder face",
        [this, state]() {
            GaugeFace* current = state->face;
            if (!current) return false;
            faces.erase(faces.begin() + static_cast<std::ptrdiff_t>(state->from));
            faces.insert(faces.begin() + static_cast<std::ptrdiff_t>(state->to), current);
            return true;
        },
        [this, state]() {
            GaugeFace* current = state->face;
            if (!current) return false;
            faces.erase(faces.begin() + static_cast<std::ptrdiff_t>(state->to));
            faces.insert(faces.begin() + static_cast<std::ptrdiff_t>(state->from), current);
            return true;
        }
    });

    return committed ? OkObject() : Error("Failed to reorder face");
}

EditorResult Editor::createElement(const ElementPlacement& where, const std::string& json) {
    ElementContainerRef parent = getElementContainerById(where.parentId);
    if (!parent.isFace() && !parent.isElement()) return Error("Invalid parent id");

    rapidjson::Document doc = parseJson(json);
    if (!doc.IsObject()) return Error("Invalid JSON");

    const std::size_t index = clampIndex(where.index, childCountOf(parent));
    auto state = std::make_shared<ElementCreateState>();
    state->parentId = where.parentId;
    state->index = index;
    state->json = json;

    const bool committed = history.commit({
        "create element",
        [this, state]() {
            ElementContainerRef currentParent = getElementContainerById(state->parentId);
            if (!currentParent.isFace() && !currentParent.isElement()) return false;

            rapidjson::Document parsed = parseJson(state->json);
            const rapidjson::Value& parsedValue = parsed;
            OwnedElement element = Element::fromJson(parsedValue.GetObject());
            Element* raw = element.get();
            if (!insertIntoContainer(currentParent, std::move(element), state->index)) return false;

            if (!state->initialized) {
                registerSubtree(raw);
                state->created = snapshotElement(*this, *raw);
                state->initialized = true;
                return true;
            }

            std::size_t next = 0;
            if (!registerSubtreeWithIds(raw, state->created.ids, next)) return false;
            return next == state->created.ids.size();
        },
        [this, state]() {
            if (state->created.ids.empty()) return false;
            ElementContainerRef currentParent = getElementContainerById(state->parentId);
            Element* current = getElementById(state->created.ids.front());
            if ((!currentParent.isFace() && !currentParent.isElement()) || !current) return false;
            unregisterElementRecursive(current);
            return static_cast<bool>(removeFromContainer(currentParent, current));
        }
    });

    return committed ? okWithIds(state->created.ids.front(), where.parentId) : Error("Failed to create element");
}

EditorResult Editor::removeElement(Id elementId) {
    Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");

    ElementContainerRef parent = getElementContainerOf(element);
    if (!parent.isFace() && !parent.isElement()) return Error("Element is not attached to a parent");

    const std::size_t index = indexInContainer(parent, element);
    auto state = std::make_shared<ElementRemoveState>();
    state->parentId = idOfContainer(parent);
    state->index = index;
    state->removed = snapshotElement(*this, *element);

    const bool committed = history.commit({
        "remove element",
        [this, state]() {
            ElementContainerRef currentParent = getElementContainerById(state->parentId);
            if ((!currentParent.isFace() && !currentParent.isElement()) || state->removed.ids.empty()) return false;
            Element* current = getElementById(state->removed.ids.front());
            if (!current) return false;
            unregisterElementRecursive(current);
            return static_cast<bool>(removeFromContainer(currentParent, current));
        },
        [this, state]() {
            ElementContainerRef currentParent = getElementContainerById(state->parentId);
            if (!currentParent.isFace() && !currentParent.isElement()) return false;
            rapidjson::Document parsed = parseJson(state->removed.json);
            const rapidjson::Value& parsedValue = parsed;
            OwnedElement restored = Element::fromJson(parsedValue.GetObject());
            Element* raw = restored.get();
            if (!insertIntoContainer(currentParent, std::move(restored), state->index)) return false;
            std::size_t next = 0;
            if (!registerSubtreeWithIds(raw, state->removed.ids, next)) return false;
            return next == state->removed.ids.size();
        }
    });

    return committed ? OkObject() : Error("Failed to remove element");
}

EditorResult Editor::reorderElement(Id elementId, std::size_t index) {
    Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");

    ElementContainerRef parent = getElementContainerOf(element);
    if (!parent.isFace() && !parent.isElement()) return Error("Element is not attached to a parent");

    const std::size_t oldIndex = indexInContainer(parent, element);
    const std::size_t newIndex = clampIndex(index, childCountOf(parent) - 1);
    if (oldIndex == newIndex) return OkObject();

    auto state = std::make_shared<ElementReorderState>();
    state->parentId = idOfContainer(parent);
    state->elementId = elementId;
    state->from = oldIndex;
    state->to = newIndex;

    const bool committed = history.commit({
        "reorder element",
        [this, state]() {
            ElementContainerRef currentParent = getElementContainerById(state->parentId);
            Element* current = getElementById(state->elementId);
            if ((!currentParent.isFace() && !currentParent.isElement()) || !current) return false;
            OwnedElement owned = removeFromContainer(currentParent, current);
            return owned && insertIntoContainer(currentParent, std::move(owned), state->to);
        },
        [this, state]() {
            ElementContainerRef currentParent = getElementContainerById(state->parentId);
            Element* current = getElementById(state->elementId);
            if ((!currentParent.isFace() && !currentParent.isElement()) || !current) return false;
            OwnedElement owned = removeFromContainer(currentParent, current);
            return owned && insertIntoContainer(currentParent, std::move(owned), state->from);
        }
    });

    return committed ? OkObject() : Error("Failed to reorder element");
}

EditorResult Editor::moveElement(Id elementId, const ElementPlacement& where) {
    Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");

    ElementContainerRef oldParent = getElementContainerOf(element);
    ElementContainerRef newParent = getElementContainerById(where.parentId);
    if (!oldParent.isFace() && !oldParent.isElement()) return Error("Element is not attached to a parent");
    if (!newParent.isFace() && !newParent.isElement()) return Error("Invalid destination parent id");
    if (idOfContainer(oldParent) == where.parentId) return Error("Destination parent matches current parent; use reorderElement");
    if (newParent.element && elementContains(element, newParent.element)) return Error("Cannot move an element into its own subtree");

    const std::size_t fromIndex = indexInContainer(oldParent, element);
    const std::size_t toIndex = clampIndex(where.index, childCountOf(newParent));

    auto state = std::make_shared<ElementMoveState>();
    state->elementId = elementId;
    state->oldParentId = idOfContainer(oldParent);
    state->newParentId = where.parentId;
    state->fromIndex = fromIndex;
    state->toIndex = toIndex;

    const bool committed = history.commit({
        "move element",
        [this, state]() {
            ElementContainerRef oldParentCurrent = getElementContainerById(state->oldParentId);
            ElementContainerRef newParentCurrent = getElementContainerById(state->newParentId);
            Element* current = getElementById(state->elementId);
            if ((!oldParentCurrent.isFace() && !oldParentCurrent.isElement()) ||
                (!newParentCurrent.isFace() && !newParentCurrent.isElement()) ||
                !current) {
                return false;
            }
            OwnedElement owned = removeFromContainer(oldParentCurrent, current);
            return owned && insertIntoContainer(newParentCurrent, std::move(owned), state->toIndex);
        },
        [this, state]() {
            ElementContainerRef oldParentCurrent = getElementContainerById(state->oldParentId);
            ElementContainerRef newParentCurrent = getElementContainerById(state->newParentId);
            Element* current = getElementById(state->elementId);
            if ((!oldParentCurrent.isFace() && !oldParentCurrent.isElement()) ||
                (!newParentCurrent.isFace() && !newParentCurrent.isElement()) ||
                !current) {
                return false;
            }
            OwnedElement owned = removeFromContainer(newParentCurrent, current);
            return owned && insertIntoContainer(oldParentCurrent, std::move(owned), state->fromIndex);
        }
    });

    return committed ? okWithIds(elementId, where.parentId) : Error("Failed to move element");
}

EditorResult Editor::replaceElementFromJson(Id elementId, const std::string& json) {
    Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");

    rapidjson::Document doc = parseJson(json);
    if (!doc.IsObject()) return Error("Invalid JSON");

    auto state = std::make_shared<ReplaceState>();
    state->targetId = elementId;
    state->pendingJson = json;
    state->before = snapshotElement(*this, *element);
    state->parentId = element->getParent() ? idOf(element->getParent()) : 0;

    if (element->getParent()) {
        state->index = indexOfChild(*element->getParent(), element);
    } else {
        GaugeFace* ownerFace = findOwningFace(faces, element);
        if (!ownerFace) return Error("Root element is not attached to a face");
        state->faceId = idOf(ownerFace);
        state->index = indexOfRoot(*ownerFace, element);
    }

    const bool committed = history.commit({
        "replace element",
        [this, state]() {
            Element* current = getElementById(state->targetId);
            if (!current) return false;
            unregisterElementRecursive(current);

            if (state->parentId != 0) {
                Element* parent = getElementById(state->parentId);
                if (!parent || !parent->removeChild(current)) return false;
            } else {
                GaugeFace* face = getFaceById(state->faceId);
                if (!face || !face->removeChild(current)) return false;
            }

            rapidjson::Document parsed = parseJson(state->initialized ? state->after.json : state->pendingJson);
            const rapidjson::Value& parsedValue = parsed;
            OwnedElement replacement = Element::fromJson(parsedValue.GetObject());
            Element* raw = replacement.get();

            const bool inserted = state->parentId != 0
                ? static_cast<bool>(getElementById(state->parentId) && getElementById(state->parentId)->insertChild(std::move(replacement), state->index))
                : static_cast<bool>(getFaceById(state->faceId) && getFaceById(state->faceId)->insertChild(std::move(replacement), state->index));
            if (!inserted) return false;

            if (!state->initialized) {
                registerElementWithId(state->targetId, raw);
                for (std::size_t i = 0; i < raw->childCount(); ++i) registerSubtree(raw->childAt(i));
                state->after = snapshotElement(*this, *raw);
                state->initialized = true;
                return true;
            }

            std::size_t next = 0;
            if (!registerSubtreeWithIds(raw, state->after.ids, next)) return false;
            return next == state->after.ids.size();
        },
        [this, state]() {
            Element* current = getElementById(state->targetId);
            if (!current) return false;
            unregisterElementRecursive(current);

            if (state->parentId != 0) {
                Element* parent = getElementById(state->parentId);
                if (!parent || !parent->removeChild(current)) return false;
            } else {
                GaugeFace* face = getFaceById(state->faceId);
                if (!face || !face->removeChild(current)) return false;
            }

            rapidjson::Document parsed = parseJson(state->before.json);
            const rapidjson::Value& parsedValue = parsed;
            OwnedElement restored = Element::fromJson(parsedValue.GetObject());
            Element* raw = restored.get();

            const bool inserted = state->parentId != 0
                ? static_cast<bool>(getElementById(state->parentId) && getElementById(state->parentId)->insertChild(std::move(restored), state->index))
                : static_cast<bool>(getFaceById(state->faceId) && getFaceById(state->faceId)->insertChild(std::move(restored), state->index));
            if (!inserted) return false;

            std::size_t next = 0;
            if (!registerSubtreeWithIds(raw, state->before.ids, next)) return false;
            return next == state->before.ids.size();
        }
    });

    return committed ? okWithId(state->targetId) : Error("Failed to replace element");
}

EditorResult Editor::setProperty(Id id, const std::string& path, const std::string& json) {
    PropertyObject* object = getObjectById(id);
    if (!object) return Error("Invalid id");

    rapidjson::Document value = parseJson(json);
    if (value.HasParseError()) return Error("Invalid JSON");

    PropertyObject* owner = nullptr;
    const Property* property = nullptr;
    if (!object->resolvePath(path, owner, property) || !owner || !property) {
        return Error("Invalid property path");
    }

    rapidjson::Document beforeDoc;
    beforeDoc.SetObject();
    if (!owner->getProperty(property->key, beforeDoc, beforeDoc.GetAllocator())) {
        return Error("Failed to read current property value");
    }

    const std::string beforeJson = toString(beforeDoc);
    auto state = std::make_shared<PropertySetState>();
    state->nodeId = id;
    state->path = path;
    state->beforeJson = beforeJson;
    state->afterJson = json;

    const bool committed = history.commit({
        "set property",
        [this, state]() {
            PropertyObject* currentObject = getObjectById(state->nodeId);
            if (!currentObject) return false;
            PropertyObject* currentOwner = nullptr;
            const Property* currentProperty = nullptr;
            if (!currentObject->resolvePath(state->path, currentOwner, currentProperty) || !currentOwner || !currentProperty) return false;
            rapidjson::Document parsed = parseJson(state->afterJson);
            return currentOwner->setProperty(currentProperty->key, parsed);
        },
        [this, state]() {
            PropertyObject* currentObject = getObjectById(state->nodeId);
            if (!currentObject) return false;
            PropertyObject* currentOwner = nullptr;
            const Property* currentProperty = nullptr;
            if (!currentObject->resolvePath(state->path, currentOwner, currentProperty) || !currentOwner || !currentProperty) return false;
            rapidjson::Document parsed = parseJson(state->beforeJson);
            return currentOwner->setProperty(currentProperty->key, parsed);
        }
    });

    return committed ? OkObject() : Error("Failed to set property");
}

EditorResult Editor::getProperty(Id id, const std::string& path) const {
    const NodeRef* node = getNode(id);
    if (!node) return Error("Invalid id");
    if (path.empty()) return Error("Property path is required");

    const PropertyObject* object = node->kind == NodeKind::Face ? static_cast<const PropertyObject*>(node->face) : static_cast<const PropertyObject*>(node->element);

    const PropertyObject* owner = nullptr;
    const Property* property = nullptr;
    if (!object->resolvePath(path, owner, property) || !owner || !property) return Error("Invalid property path");

    EditorResult result = OkObject();
    rapidjson::Value value;
    if (!owner->getProperty(property->key, value, result.data.GetAllocator())) return Error("Failed to read property");

    result.data.AddMember("id", id, result.data.GetAllocator());
    result.data.AddMember("path", rapidjson::Value(path.c_str(), result.data.GetAllocator()), result.data.GetAllocator());
    result.data.AddMember("value", std::move(value), result.data.GetAllocator());
    return result;
}

EditorResult Editor::getPropertiesMeta(Id id, const std::string& path) const {
    const NodeRef* node = getNode(id);
    if (!node) return Error("Invalid id");

    const PropertyObject* object = node->kind == NodeKind::Face ? static_cast<const PropertyObject*>(node->face) : static_cast<const PropertyObject*>(node->element);

    EditorResult result = OkObject();
    auto& allocator = result.data.GetAllocator();
    result.data.AddMember("id", id, allocator);

    if (path.empty()) {
        rapidjson::Value meta = object->getPropertiesMeta(allocator);
        result.data.AddMember("meta", std::move(meta), allocator);
        return result;
    }

    const PropertyObject* owner = nullptr;
    const Property* property = nullptr;
    if (!object->resolvePath(path, owner, property) || !owner || !property) return Error("Invalid property path");

    rapidjson::Value meta = owner->getPropertyMeta(*property, allocator);
    result.data.AddMember("path", rapidjson::Value(path.c_str(), allocator), allocator);
    result.data.AddMember("meta", std::move(meta), allocator);
    return result;
}

EditorResult Editor::copyFace(Id faceId) {
    const GaugeFace* face = getFaceById(faceId);
    if (!face) return Error("Invalid face id");

    clipboard.kind = ClipboardState::Kind::Face;
    clipboard.json = toString(face->save());
    return OkObject();
}

EditorResult Editor::cutFace(Id faceId) {
    EditorResult copied = copyFace(faceId);
    if (!copied.ok) return copied;
    return removeFace(faceId);
}

EditorResult Editor::pasteFace(FacePlacement where) {
    if (clipboard.kind != ClipboardState::Kind::Face) return Error("Clipboard does not contain a face");

    rapidjson::Document parsed = parseJson(clipboard.json);
    if (!parsed.IsObject()) return Error("Clipboard face payload is invalid");

    auto face = std::make_unique<GaugeFace>();
    face->load(parsed);

    GaugeFace* raw = face.get();
    ownedFaces.push_back(std::move(face));
    return insertFace(*raw, where);
}

EditorResult Editor::copyElement(Id elementId) {
    const Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");

    clipboard.kind = ClipboardState::Kind::Element;
    clipboard.json = serializeElement(*element);
    return OkObject();
}

EditorResult Editor::cutElement(Id elementId) {
    EditorResult copied = copyElement(elementId);
    if (!copied.ok) return copied;
    return removeElement(elementId);
}

EditorResult Editor::pasteElement(const ElementPlacement& where) {
    if (clipboard.kind != ClipboardState::Kind::Element) return Error("Clipboard does not contain an element");
    return createElement(where, clipboard.json);
}

EditorResult Editor::pasteToReplaceElement(Id elementId) {
    if (clipboard.kind != ClipboardState::Kind::Element) return Error("Clipboard does not contain an element");
    return replaceElementFromJson(elementId, clipboard.json);
}

EditorResult Editor::getHistory() const {
    EditorResult result = OkObject();
    auto& allocator = result.data.GetAllocator();
    result.data.AddMember("canUndo", history.canUndo(), allocator);
    result.data.AddMember("canRedo", history.canRedo(), allocator);
    return result;
}
