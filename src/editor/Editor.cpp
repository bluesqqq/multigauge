#include <multigauge/editor/Editor.h>

#include <algorithm>
#include <memory>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

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

struct RootCreateState {
    Editor::Id faceId = 0;
    std::size_t index = 0;
    std::string json;
    ElementSnapshot created;
    bool initialized = false;
};

struct RootRemoveState {
    Editor::Id faceId = 0;
    std::size_t index = 0;
    ElementSnapshot removed;
};

struct RootReorderState {
    Editor::Id faceId = 0;
    Editor::Id rootId = 0;
    std::size_t from = 0;
    std::size_t to = 0;
};

struct RootMoveState {
    Editor::Id rootId = 0;
    Editor::Id fromFaceId = 0;
    Editor::Id toFaceId = 0;
    std::size_t fromIndex = 0;
    std::size_t toIndex = 0;
};

struct ChildCreateState {
    Editor::Id parentId = 0;
    std::size_t index = 0;
    std::string json;
    ElementSnapshot created;
    bool initialized = false;
};

struct ChildRemoveState {
    Editor::Id parentId = 0;
    std::size_t index = 0;
    ElementSnapshot removed;
};

struct ChildReorderState {
    Editor::Id parentId = 0;
    Editor::Id elementId = 0;
    std::size_t from = 0;
    std::size_t to = 0;
};

struct ChildMoveState {
    Editor::Id elementId = 0;
    Editor::Id oldParentId = 0;
    Editor::Id newParentId = 0;
    std::size_t oldIndex = 0;
    std::size_t newIndex = 0;
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

EditorResult Editor::listFaces() const {
    EditorResult result = OkArray();
    auto& allocator = result.data.GetAllocator();

    for (std::size_t i = 0; i < faces.size(); ++i) {
        const GaugeFace* face = faces[i];
        if (!face) continue;

        rapidjson::Value item(rapidjson::kObjectType);
        item.AddMember("id", idOf(face), allocator);
        item.AddMember("index", static_cast<uint32_t>(i), allocator);
        addString(item, "kind", "face", allocator);
        item.AddMember("rootCount", static_cast<uint32_t>(face->childCount()), allocator);
        result.data.PushBack(std::move(item), allocator);
    }

    return result;
}

EditorResult Editor::listRoots(Id faceId) const {
    const GaugeFace* face = getFaceById(faceId);
    if (!face) return Error("Invalid face id");

    EditorResult result = OkArray();
    auto& allocator = result.data.GetAllocator();

    for (std::size_t i = 0; i < face->childCount(); ++i) {
        const Element* element = face->childAt(i);
        rapidjson::Value item(rapidjson::kObjectType);
        item.AddMember("id", idOf(element), allocator);
        item.AddMember("index", static_cast<uint32_t>(i), allocator);
        addString(item, "kind", "element", allocator);
        item.AddMember("faceId", faceId, allocator);
        item.AddMember("childCount", static_cast<uint32_t>(element->childCount()), allocator);
        result.data.PushBack(std::move(item), allocator);
    }

    return result;
}

EditorResult Editor::listChildren(Id elementId) const {
    const Element* parent = getElementById(elementId);
    if (!parent) return Error("Invalid element id");

    EditorResult result = OkArray();
    auto& allocator = result.data.GetAllocator();

    for (std::size_t i = 0; i < parent->childCount(); ++i) {
        const Element* child = parent->childAt(i);
        rapidjson::Value item(rapidjson::kObjectType);
        item.AddMember("id", idOf(child), allocator);
        item.AddMember("index", static_cast<uint32_t>(i), allocator);
        addString(item, "kind", "element", allocator);
        item.AddMember("parentElementId", elementId, allocator);
        item.AddMember("childCount", static_cast<uint32_t>(child->childCount()), allocator);
        result.data.PushBack(std::move(item), allocator);
    }

    return result;
}

EditorResult Editor::describeNode(Id id) const {
    const NodeRef* node = getNode(id);
    if (!node) return Error("Invalid id");

    EditorResult result = OkObject();
    auto& allocator = result.data.GetAllocator();
    result.data.AddMember("id", id, allocator);

    if (node->kind == NodeKind::Face) {
        GaugeFace* face = node->face;
        addString(result.data, "kind", "face", allocator);
        result.data.AddMember("rootCount", static_cast<uint32_t>(face->childCount()), allocator);
        result.data.AddMember("index", static_cast<uint32_t>(
            std::distance(faces.begin(), std::find(faces.begin(), faces.end(), face))), allocator);
        return result;
    }

    Element* element = node->element;
    addString(result.data, "kind", "element", allocator);
    result.data.AddMember("childCount", static_cast<uint32_t>(element->childCount()), allocator);

    if (Element* parent = element->getParent()) {
        result.data.AddMember("parentElementId", idOf(parent), allocator);
        result.data.AddMember("index", static_cast<uint32_t>(indexOfChild(*parent, element)), allocator);
    } else if (GaugeFace* face = findOwningFace(faces, element)) {
        result.data.AddMember("faceId", idOf(face), allocator);
        result.data.AddMember("index", static_cast<uint32_t>(indexOfRoot(*face, element)), allocator);
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

EditorResult Editor::createRoot(const RootPlacement& where, const std::string& json) {
    GaugeFace* face = getFaceById(where.faceId);
    if (!face) return Error("Invalid face id");

    rapidjson::Document doc = parseJson(json);
    if (!doc.IsObject()) return Error("Invalid JSON");

    const std::size_t index = clampIndex(where.index, face->childCount());
    auto state = std::make_shared<RootCreateState>();
    state->faceId = where.faceId;
    state->index = index;
    state->json = json;

    const bool committed = history.commit({
        "create root",
        [this, state]() {
            GaugeFace* currentFace = getFaceById(state->faceId);
            if (!currentFace) return false;

            rapidjson::Document parsed = parseJson(state->json);
            const rapidjson::Value& parsedValue = parsed;
            OwnedElement element = Element::fromJson(parsedValue.GetObject());
            Element* raw = element.get();
            if (!currentFace->insertChild(std::move(element), state->index)) {
                return false;
            }
            
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
            GaugeFace* currentFace = getFaceById(state->faceId);
            Element* current = getElementById(state->created.ids.front());
            if (!currentFace || !current) return false;
            unregisterElementRecursive(current);
            return static_cast<bool>(currentFace->removeChild(current));
        }
    });

    return committed ? okWithIds(state->created.ids.front(), where.faceId) : Error("Failed to create root");
}

EditorResult Editor::removeRoot(Id elementId) {
    Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");
    if (element->getParent()) return Error("Element is not a root; use removeElement");

    GaugeFace* face = findOwningFace(faces, element);
    if (!face) return Error("Root element is not attached to a face");

    const std::size_t index = indexOfRoot(*face, element);
    auto state = std::make_shared<RootRemoveState>();
    state->faceId = idOf(face);
    state->index = index;
    state->removed = snapshotElement(*this, *element);

    const bool committed = history.commit({
        "remove root",
        [this, state]() {
            GaugeFace* currentFace = getFaceById(state->faceId);
            if (!currentFace || state->removed.ids.empty()) return false;
            Element* current = getElementById(state->removed.ids.front());
            if (!current) return false;
            unregisterElementRecursive(current);
            return static_cast<bool>(currentFace->removeChild(current));
        },
        [this, state]() {
            GaugeFace* currentFace = getFaceById(state->faceId);
            if (!currentFace) return false;
            rapidjson::Document parsed = parseJson(state->removed.json);
            const rapidjson::Value& parsedValue = parsed;
            OwnedElement restored = Element::fromJson(parsedValue.GetObject());
            Element* raw = restored.get();
            if (!currentFace->insertChild(std::move(restored), state->index)) return false;
            std::size_t next = 0;
            if (!registerSubtreeWithIds(raw, state->removed.ids, next)) return false;
            return next == state->removed.ids.size();
        }
    });

    return committed ? OkObject() : Error("Failed to remove root");
}

EditorResult Editor::reorderRoot(Id elementId, std::size_t index) {
    Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");
    if (element->getParent()) return Error("Element is not a root; use reorderChild");

    GaugeFace* face = findOwningFace(faces, element);
    if (!face) return Error("Root element is not attached to a face");

    const std::size_t oldIndex = indexOfRoot(*face, element);
    const std::size_t newIndex = clampIndex(index, face->childCount() - 1);
    if (oldIndex == newIndex) return OkObject();

    auto state = std::make_shared<RootReorderState>();
    state->faceId = idOf(face);
    state->rootId = elementId;
    state->from = oldIndex;
    state->to = newIndex;

    const bool committed = history.commit({
        "reorder root",
        [this, state]() {
            GaugeFace* currentFace = getFaceById(state->faceId);
            Element* current = getElementById(state->rootId);
            if (!currentFace || !current) return false;
            OwnedElement owned = currentFace->removeChild(current);
            return owned && currentFace->insertChild(std::move(owned), state->to);
        },
        [this, state]() {
            GaugeFace* currentFace = getFaceById(state->faceId);
            Element* current = getElementById(state->rootId);
            if (!currentFace || !current) return false;
            OwnedElement owned = currentFace->removeChild(current);
            return owned && currentFace->insertChild(std::move(owned), state->from);
        }
    });

    return committed ? OkObject() : Error("Failed to reorder root");
}

EditorResult Editor::moveRootToFace(Id elementId, const RootPlacement& where) {
    Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");
    if (element->getParent()) return Error("Element is not a root; use moveElementToParent");

    GaugeFace* fromFace = findOwningFace(faces, element);
    GaugeFace* toFace = getFaceById(where.faceId);
    if (!fromFace) return Error("Root element is not attached to a face");
    if (!toFace) return Error("Invalid destination face id");
    if (fromFace == toFace) return Error("Destination face matches current face; use reorderRoot");

    const std::size_t fromIndex = indexOfRoot(*fromFace, element);
    const std::size_t toIndex = clampIndex(where.index, toFace->childCount());

    auto state = std::make_shared<RootMoveState>();
    state->rootId = elementId;
    state->fromFaceId = idOf(fromFace);
    state->toFaceId = where.faceId;
    state->fromIndex = fromIndex;
    state->toIndex = toIndex;

    const bool committed = history.commit({
        "move root",
        [this, state]() {
            GaugeFace* from = getFaceById(state->fromFaceId);
            GaugeFace* to = getFaceById(state->toFaceId);
            Element* current = getElementById(state->rootId);
            if (!from || !to || !current) return false;
            OwnedElement owned = from->removeChild(current);
            return owned && to->insertChild(std::move(owned), state->toIndex);
        },
        [this, state]() {
            GaugeFace* from = getFaceById(state->fromFaceId);
            GaugeFace* to = getFaceById(state->toFaceId);
            Element* current = getElementById(state->rootId);
            if (!from || !to || !current) return false;
            OwnedElement owned = to->removeChild(current);
            return owned && from->insertChild(std::move(owned), state->fromIndex);
        }
    });

    return committed ? okWithIds(elementId, where.faceId) : Error("Failed to move root");
}

EditorResult Editor::createChild(const ChildPlacement& where, const std::string& json) {
    Element* parent = getElementById(where.parentElementId);
    if (!parent) return Error("Invalid parent element id");

    rapidjson::Document doc = parseJson(json);
    if (!doc.IsObject()) return Error("Invalid JSON");

    const std::size_t index = clampIndex(where.index, parent->childCount());
    auto state = std::make_shared<ChildCreateState>();
    state->parentId = where.parentElementId;
    state->index = index;
    state->json = json;

    const bool committed = history.commit({
        "create child",
        [this, state]() {
            Element* currentParent = getElementById(state->parentId);
            if (!currentParent) return false;

            rapidjson::Document parsed = parseJson(state->json);
            const rapidjson::Value& parsedValue = parsed;
            OwnedElement element = Element::fromJson(parsedValue.GetObject());
            Element* raw = element.get();
            if (!currentParent->insertChild(std::move(element), state->index)) {
                return false;
            }
            
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
            Element* currentParent = getElementById(state->parentId);
            Element* current = getElementById(state->created.ids.front());
            if (!currentParent || !current) return false;
            unregisterElementRecursive(current);
            return static_cast<bool>(currentParent->removeChild(current));
        }
    });

    return committed ? okWithIds(state->created.ids.front(), where.parentElementId) : Error("Failed to create child");
}

EditorResult Editor::removeElement(Id elementId) {
    Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");

    Element* parent = element->getParent();
    if (!parent) return Error("Element is a root; use removeRoot");

    const std::size_t index = indexOfChild(*parent, element);
    auto state = std::make_shared<ChildRemoveState>();
    state->parentId = idOf(parent);
    state->index = index;
    state->removed = snapshotElement(*this, *element);

    const bool committed = history.commit({
        "remove element",
        [this, state]() {
            Element* currentParent = getElementById(state->parentId);
            if (!currentParent || state->removed.ids.empty()) return false;
            Element* current = getElementById(state->removed.ids.front());
            if (!current) return false;
            unregisterElementRecursive(current);
            return static_cast<bool>(currentParent->removeChild(current));
        },
        [this, state]() {
            Element* currentParent = getElementById(state->parentId);
            if (!currentParent) return false;
            rapidjson::Document parsed = parseJson(state->removed.json);
            const rapidjson::Value& parsedValue = parsed;
            OwnedElement restored = Element::fromJson(parsedValue.GetObject());
            Element* raw = restored.get();
            if (!currentParent->insertChild(std::move(restored), state->index)) return false;
            std::size_t next = 0;
            if (!registerSubtreeWithIds(raw, state->removed.ids, next)) return false;
            return next == state->removed.ids.size();
        }
    });

    return committed ? OkObject() : Error("Failed to remove element");
}

EditorResult Editor::reorderChild(Id elementId, std::size_t index) {
    Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");

    Element* parent = element->getParent();
    if (!parent) return Error("Element is a root; use reorderRoot");

    const std::size_t oldIndex = indexOfChild(*parent, element);
    const std::size_t newIndex = clampIndex(index, parent->childCount() - 1);
    if (oldIndex == newIndex) return OkObject();

    auto state = std::make_shared<ChildReorderState>();
    state->parentId = idOf(parent);
    state->elementId = elementId;
    state->from = oldIndex;
    state->to = newIndex;

    const bool committed = history.commit({
        "reorder child",
        [this, state]() {
            Element* currentParent = getElementById(state->parentId);
            Element* current = getElementById(state->elementId);
            if (!currentParent || !current) return false;
            OwnedElement owned = currentParent->removeChild(current);
            return owned && currentParent->insertChild(std::move(owned), state->to);
        },
        [this, state]() {
            Element* currentParent = getElementById(state->parentId);
            Element* current = getElementById(state->elementId);
            if (!currentParent || !current) return false;
            OwnedElement owned = currentParent->removeChild(current);
            return owned && currentParent->insertChild(std::move(owned), state->from);
        }
    });

    return committed ? OkObject() : Error("Failed to reorder child");
}

EditorResult Editor::moveElementToParent(Id elementId, const ChildPlacement& where) {
    Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");

    Element* oldParent = element->getParent();
    if (!oldParent) return Error("Element is a root; use moveRootToFace");

    Element* newParent = getElementById(where.parentElementId);
    if (!newParent) return Error("Invalid destination parent element id");
    if (newParent == oldParent) return Error("Destination parent matches current parent; use reorderChild");
    if (elementContains(element, newParent)) return Error("Cannot move an element into its own subtree");

    const std::size_t oldIndex = indexOfChild(*oldParent, element);
    const std::size_t newIndex = clampIndex(where.index, newParent->childCount());
    auto state = std::make_shared<ChildMoveState>();
    state->elementId = elementId;
    state->oldParentId = idOf(oldParent);
    state->newParentId = where.parentElementId;
    state->oldIndex = oldIndex;
    state->newIndex = newIndex;

    const bool committed = history.commit({
        "move element",
        [this, state]() {
            Element* oldParentCurrent = getElementById(state->oldParentId);
            Element* newParentCurrent = getElementById(state->newParentId);
            Element* current = getElementById(state->elementId);
            if (!oldParentCurrent || !newParentCurrent || !current) return false;
            OwnedElement owned = oldParentCurrent->removeChild(current);
            return owned && newParentCurrent->insertChild(std::move(owned), state->newIndex);
        },
        [this, state]() {
            Element* oldParentCurrent = getElementById(state->oldParentId);
            Element* newParentCurrent = getElementById(state->newParentId);
            Element* current = getElementById(state->elementId);
            if (!oldParentCurrent || !newParentCurrent || !current) return false;
            OwnedElement owned = newParentCurrent->removeChild(current);
            return owned && oldParentCurrent->insertChild(std::move(owned), state->oldIndex);
        }
    });

    return committed ? okWithIds(elementId, where.parentElementId) : Error("Failed to move element");
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
                for (std::size_t i = 0; i < raw->childCount(); ++i) {
                    registerSubtree(raw->childAt(i));
                }
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
            if (!currentObject->resolvePath(state->path, currentOwner, currentProperty) || !currentOwner || !currentProperty) {
                return false;
            }
            rapidjson::Document parsed = parseJson(state->afterJson);
            return currentOwner->setProperty(currentProperty->key, parsed);
        },
        [this, state]() {
            PropertyObject* currentObject = getObjectById(state->nodeId);
            if (!currentObject) return false;
            PropertyObject* currentOwner = nullptr;
            const Property* currentProperty = nullptr;
            if (!currentObject->resolvePath(state->path, currentOwner, currentProperty) || !currentOwner || !currentProperty) {
                return false;
            }
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

    const PropertyObject* object = node->kind == NodeKind::Face
        ? static_cast<const PropertyObject*>(node->face)
        : static_cast<const PropertyObject*>(node->element);

    const PropertyObject* owner = nullptr;
    const Property* property = nullptr;
    if (!object->resolvePath(path, owner, property) || !owner || !property) {
        return Error("Invalid property path");
    }

    EditorResult result = OkObject();
    rapidjson::Value value;
    if (!owner->getProperty(property->key, value, result.data.GetAllocator())) {
        return Error("Failed to read property");
    }

    result.data.AddMember("id", id, result.data.GetAllocator());
    result.data.AddMember("path", rapidjson::Value(path.c_str(), result.data.GetAllocator()), result.data.GetAllocator());
    result.data.AddMember("value", std::move(value), result.data.GetAllocator());
    return result;
}

EditorResult Editor::getPropertiesMeta(Id id, const std::string& path) const {
    const NodeRef* node = getNode(id);
    if (!node) return Error("Invalid id");

    const PropertyObject* object = node->kind == NodeKind::Face
        ? static_cast<const PropertyObject*>(node->face)
        : static_cast<const PropertyObject*>(node->element);

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
    if (!object->resolvePath(path, owner, property) || !owner || !property) {
        return Error("Invalid property path");
    }

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

EditorResult Editor::cutRoot(Id elementId) {
    EditorResult copied = copyElement(elementId);
    if (!copied.ok) return copied;
    return removeRoot(elementId);
}

EditorResult Editor::cutElement(Id elementId) {
    EditorResult copied = copyElement(elementId);
    if (!copied.ok) return copied;
    return removeElement(elementId);
}

EditorResult Editor::pasteRoot(const RootPlacement& where) {
    if (clipboard.kind != ClipboardState::Kind::Element) return Error("Clipboard does not contain an element");
    return createRoot(where, clipboard.json);
}

EditorResult Editor::pasteChild(const ChildPlacement& where) {
    if (clipboard.kind != ClipboardState::Kind::Element) return Error("Clipboard does not contain an element");
    return createChild(where, clipboard.json);
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
