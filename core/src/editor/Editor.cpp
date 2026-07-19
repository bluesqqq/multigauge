#include <multigauge/editor/Editor.h>

#include <algorithm>
#include <cstdint>
#include <memory>

#include <rapidjson/document.h>

#include <multigauge/value/Value.h>
#include <multigauge/utils/Json.h>

namespace mg::editor {

using ::mg::Result;

namespace {

struct FaceInsertState {
    GaugeFace* face = nullptr;
    std::size_t index = 0;
    NodeId faceId = 0;
    Editor::FaceMeta meta;
    std::vector<std::vector<NodeId>> rootIds;
    bool initialized = false;
};

struct FaceRemoveState {
    GaugeFace* face = nullptr;
    NodeId faceId = 0;
    std::size_t index = 0;
    Editor::FaceMeta meta;
    std::string json;
    std::vector<std::vector<NodeId>> rootIds;
};

struct FaceReorderState {
    GaugeFace* face = nullptr;
    NodeId faceId = 0;
    std::size_t from = 0;
    std::size_t to = 0;
};

struct ElementSnapshot {
    std::string json;
    std::vector<NodeId> ids;
};

struct ElementCreateState {
    NodeId parentId = 0;
    std::size_t index = 0;
    std::string json;
    ElementSnapshot created;
    bool initialized = false;
};

struct ElementRemoveState {
    NodeId parentId = 0;
    std::size_t index = 0;
    ElementSnapshot removed;
};

struct ElementReorderState {
    NodeId parentId = 0;
    NodeId elementId = 0;
    std::size_t from = 0;
    std::size_t to = 0;
};

struct ElementMoveState {
    NodeId elementId = 0;
    NodeId oldParentId = 0;
    NodeId newParentId = 0;
    std::size_t fromIndex = 0;
    std::size_t toIndex = 0;
};

struct ReplaceState {
    NodeId targetId = 0;
    NodeId faceId = 0;
    NodeId parentId = 0;
    std::size_t index = 0;
    ElementSnapshot before;
    ElementSnapshot after;
    std::string pendingJson;
    bool initialized = false;
};

struct PropertySetState {
    NodeId nodeId = 0;
    std::string path;
    std::string beforeJson;
    std::string afterJson;
};

struct PackageInfoState {
    std::string beforeName;
    std::string beforeAuthor;
    std::string beforeDescription;
    std::string afterName;
    std::string afterAuthor;
    std::string afterDescription;
};

struct FaceRenameState {
    NodeId faceId = 0;
    std::string beforeName;
    std::string afterName;
};

void collectSubtreeIds(const Editor& editor, const Element* element, std::vector<NodeId>& ids) {
    if (!element) return;

    ids.push_back(editor.idOf(element));
    for (std::size_t i = 0; i < element->childCount(); ++i) {
        collectSubtreeIds(editor, element->childAt(i), ids);
    }
}

ElementSnapshot snapshotElement(const Editor& editor, const Element& element) {
    ElementSnapshot snapshot;
    rapidjson::Document doc;
    doc.SetObject();
    element.saveProperties(doc, doc.GetAllocator());
    snapshot.json = mg::json::toString(doc);
    collectSubtreeIds(editor, &element, snapshot.ids);
    return snapshot;
}

std::vector<std::vector<NodeId>> snapshotFaceRootIds(const Editor& editor, const GaugeFace& face) {
    std::vector<std::vector<NodeId>> rootIds;
    rootIds.reserve(face.childCount());

    for (std::size_t i = 0; i < face.childCount(); ++i) {
        std::vector<NodeId> ids;
        collectSubtreeIds(editor, face.childAt(i), ids);
        rootIds.push_back(std::move(ids));
    }

    return rootIds;
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

rapidjson::Value nodeIdValue(NodeId id) {
    return rapidjson::Value(static_cast<std::uint64_t>(id));
}

Result okWithId(NodeId id) {
    Result result = OkObject();
    auto& allocator = result.data.GetAllocator();
    result.data.AddMember("id", nodeIdValue(id), allocator);
    return result;
}

Result okWithIds(NodeId id, NodeId parentId) {
    Result result = OkObject();
    auto& allocator = result.data.GetAllocator();
    result.data.AddMember("id", nodeIdValue(id), allocator);
    result.data.AddMember("parentId", nodeIdValue(parentId), allocator);
    return result;
}

bool validatePackageShape(const rapidjson::Value& value) {
    (void)value;
    return true;
}

void appendElementHierarchyNode(const Editor& editor,
                                const Element& element,
                                rapidjson::Value& nodes,
                                rapidjson::Document::AllocatorType& allocator) {
    const NodeId id = editor.idOf(&element);
    rapidjson::Value nodeKey(std::to_string(id).c_str(), allocator);
    rapidjson::Value nodeValue(rapidjson::kObjectType);

    nodeValue.AddMember("kind", rapidjson::Value("element", allocator), allocator);
    nodeValue.AddMember("type", rapidjson::Value(element.typeName(), allocator), allocator);
    nodeValue.AddMember("name", rapidjson::Value(element.typeName(), allocator), allocator);

    rapidjson::Value children(rapidjson::kArrayType);
    children.Reserve(static_cast<rapidjson::SizeType>(element.childCount()), allocator);
    for (std::size_t i = 0; i < element.childCount(); ++i) {
        const Element* child = element.childAt(i);
        if (!child) continue;
        children.PushBack(nodeIdValue(editor.idOf(child)), allocator);
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

Editor::NodeRef* Editor::getNode(NodeId id) {
    auto it = nodes.find(id);
    return it == nodes.end() ? nullptr : &it->second;
}

const Editor::NodeRef* Editor::getNode(NodeId id) const {
    auto it = nodes.find(id);
    return it == nodes.end() ? nullptr : &it->second;
}

NodeId Editor::registerFace(GaugeFace* face) {
    if (!face) return 0;

    auto it = faceToId.find(face);
    if (it != faceToId.end()) return it->second;

    const NodeId id = makeId();
    return registerFaceWithId(id, face);
}

NodeId Editor::registerFaceWithId(NodeId id, GaugeFace* face) {
    if (!face || id == 0) return 0;
    if (nextId <= id) nextId = id + 1;

    nodes[id] = NodeRef(face);
    faceToId[face] = id;
    return id;
}

NodeId Editor::registerElement(Element* element) {
    if (!element) return 0;

    auto it = elementToId.find(element);
    if (it != elementToId.end()) return it->second;

    const NodeId id = makeId();
    return registerElementWithId(id, element);
}

NodeId Editor::registerElementWithId(NodeId id, Element* element) {
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

bool Editor::registerSubtreeWithIds(Element* element, const std::vector<NodeId>& ids, std::size_t& nextIndex) {
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

GaugeFace* Editor::getFaceById(NodeId id) {
    NodeRef* node = getNode(id);
    return (node && node->kind == NodeKind::Face) ? node->face : nullptr;
}

const GaugeFace* Editor::getFaceById(NodeId id) const {
    const NodeRef* node = getNode(id);
    return (node && node->kind == NodeKind::Face) ? node->face : nullptr;
}

Element* Editor::getElementById(NodeId id) {
    NodeRef* node = getNode(id);
    return (node && node->kind == NodeKind::Element) ? node->element : nullptr;
}

const Element* Editor::getElementById(NodeId id) const {
    const NodeRef* node = getNode(id);
    return (node && node->kind == NodeKind::Element) ? node->element : nullptr;
}

::mg::PropertyObject* Editor::getObjectById(NodeId id) {
    if (GaugeFace* face = getFaceById(id)) return face;
    if (Element* element = getElementById(id)) return element;
    return nullptr;
}

const ::mg::PropertyObject* Editor::getObjectById(NodeId id) const {
    if (const GaugeFace* face = getFaceById(id)) return face;
    if (const Element* element = getElementById(id)) return element;
    return nullptr;
}

Editor::ElementContainerRef Editor::getElementContainerById(NodeId id) {
    if (GaugeFace* face = getFaceById(id)) return { face, nullptr };
    if (Element* element = getElementById(id)) return { nullptr, element };
    return {};
}

Editor::ElementContainerRef Editor::getElementContainerOf(Element* element) {
    if (!element) return {};

    if (Element* parent = element->getParent())
        return { nullptr, parent };

    if (GaugeFace* face = element->getOwnerFace())
        return { face, nullptr };

    return {};
}

NodeId Editor::idOfContainer(const ElementContainerRef& container) const {
    if (container.face) return idOf(container.face);
    if (container.element) return idOf(container.element);
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

void Editor::clear() {
    faces.clear();
    faceMeta.clear();
    nodes.clear();
    faceToId.clear();
    elementToId.clear();
    nextId = 1;
    history = History();
    packageName = "Package Export";
    packageAuthor = "Unknown";
    packageDescription.clear();
}

bool Editor::setPackageInfo(const std::string& name, const std::string& author, const std::string& description) {
    if (packageName == name && packageAuthor == author && packageDescription == description) {
        return true;
    }

    auto state = std::make_shared<PackageInfoState>();
    state->beforeName = packageName;
    state->beforeAuthor = packageAuthor;
    state->beforeDescription = packageDescription;
    state->afterName = name;
    state->afterAuthor = author;
    state->afterDescription = description;

    const bool committed = history.commit({
        "set package info",
        [this, state]() {
            packageName = state->afterName;
            packageAuthor = state->afterAuthor;
            packageDescription = state->afterDescription;
            return true;
        },
        [this, state]() {
            packageName = state->beforeName;
            packageAuthor = state->beforeAuthor;
            packageDescription = state->beforeDescription;
            return true;
        }
    });

    return committed;
}

Editor::PackageInfo Editor::getPackageInfo() const {
    return { packageName, packageAuthor, packageDescription };
}

bool Editor::setFaceName(NodeId faceId, const std::string& name) {
    auto it = std::find_if(faces.begin(), faces.end(),
        [&](const auto& face) { return idOf(face.get()) == faceId; });
    if (it == faces.end()) return false;

    const std::size_t index = static_cast<std::size_t>(std::distance(faces.begin(), it));
    const std::string beforeName = index < faceMeta.size() ? faceMeta[index].name : std::string();
    if (beforeName == name) return true;

    auto state = std::make_shared<FaceRenameState>();
    state->faceId = faceId;
    state->beforeName = beforeName;
    state->afterName = name;

    const bool committed = history.commit({
        "rename face",
        [this, state]() {
            auto it = std::find_if(faces.begin(), faces.end(),
                [&](const auto& face) { return idOf(face.get()) == state->faceId; });
            if (it == faces.end()) return false;

            const std::size_t currentIndex = static_cast<std::size_t>(std::distance(faces.begin(), it));
            if (currentIndex >= faceMeta.size()) return false;
            faceMeta[currentIndex].name = state->afterName;
            return true;
        },
        [this, state]() {
            auto it = std::find_if(faces.begin(), faces.end(),
                [&](const auto& face) { return idOf(face.get()) == state->faceId; });
            if (it == faces.end()) return false;

            const std::size_t currentIndex = static_cast<std::size_t>(std::distance(faces.begin(), it));
            if (currentIndex >= faceMeta.size()) return false;
            faceMeta[currentIndex].name = state->beforeName;
            return true;
        }
    });

    return committed;
}

std::string Editor::getFaceName(NodeId faceId) const {
    auto it = std::find_if(faces.begin(), faces.end(),
        [&](const auto& face) { return idOf(face.get()) == faceId; });
    if (it == faces.end()) return {};

    const std::size_t index = static_cast<std::size_t>(std::distance(faces.begin(), it));
    if (index >= faceMeta.size()) return {};
    return faceMeta[index].name;
}

bool Editor::loadPackage(const std::string& json) {
    rapidjson::Document doc = mg::json::parseJson(json);
    if (doc.HasParseError()) return false;
    if (!doc.IsObject()) return false;

    std::string name;
    std::string author;
    std::string description;
    if (!mg::json::getStringMember(doc, "name", name) ||
        !mg::json::getStringMember(doc, "author", author) ||
        !mg::json::getStringMember(doc, "description", description)) {
        return false;
    }

    const rapidjson::Value* faces = mg::json::getArrayMember(doc, "faces");
    if (!faces) return false;

    clear();
    packageName = std::move(name);
    packageAuthor = std::move(author);
    packageDescription = std::move(description);

    for (const auto& faceEntry : faces->GetArray()) {
        if (!faceEntry.IsObject()) return false;

        std::string faceName;
        if (!mg::json::getStringMember(faceEntry, "name", faceName)) {
            return false;
        }

        const rapidjson::Value* faceJson = mg::json::getObjectMember(faceEntry, "face");
        if (!faceJson) return false;

        auto face = std::make_unique<GaugeFace>();
        face->load(*faceJson);

        GaugeFace* raw = face.get();

        this->faces.push_back(std::move(face));
        faceMeta.push_back({ std::move(faceName) });
        registerFace(raw);

        for (std::size_t i = 0; i < raw->childCount(); ++i) {
            registerSubtree(raw->childAt(i));
        }
    }

    return true;
}

std::string Editor::exportPackage() const {
    rapidjson::Document doc;
    doc.SetObject();
    auto& allocator = doc.GetAllocator();

    doc.AddMember("name", rapidjson::Value(packageName.c_str(), allocator), allocator);
    doc.AddMember("author", rapidjson::Value(packageAuthor.c_str(), allocator), allocator);
    doc.AddMember("description", rapidjson::Value(packageDescription.c_str(), allocator), allocator);

    rapidjson::Value facesOut(rapidjson::kArrayType);
    for (std::size_t i = 0; i < this->faces.size(); ++i) {
        const auto& face = this->faces[i];
        if (!face) continue;

        rapidjson::Document saved = face->save();
        rapidjson::Value entry(rapidjson::kObjectType);
        const FaceMeta meta = i < faceMeta.size() ? faceMeta[i] : FaceMeta{ "Face" };
        entry.AddMember("name", rapidjson::Value(meta.name.c_str(), allocator), allocator);
        rapidjson::Value faceValue;
        faceValue.CopyFrom(saved, allocator);
        entry.AddMember("face", std::move(faceValue), allocator);
        facesOut.PushBack(std::move(entry), allocator);
    }

    doc.AddMember("faces", std::move(facesOut), allocator);

    return mg::json::toString(doc);
}

Result Editor::serializeFace(NodeId faceId) const {
    const GaugeFace* face = getFaceById(faceId);
    if (!face) return Error("Invalid face id");
    std::string json = mg::json::toString(face->save());
    Result result = OkObject();
    result.data.AddMember("json", rapidjson::Value(json.c_str(), result.data.GetAllocator()), result.data.GetAllocator());
    return result;
}

Result Editor::serializeElement(NodeId elementId) const {
    const Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");
    rapidjson::Document doc;
    doc.SetObject();
    element->saveProperties(doc, doc.GetAllocator());
    std::string json = mg::json::toString(doc);
    Result result = OkObject();
    result.data.AddMember("json", rapidjson::Value(json.c_str(), result.data.GetAllocator()), result.data.GetAllocator());
    return result;
}

bool Editor::hasNode(NodeId id) const {
    return getNode(id) != nullptr;
}

bool Editor::isFace(NodeId id) const {
    return getFaceById(id) != nullptr;
}

bool Editor::isElement(NodeId id) const {
    return getElementById(id) != nullptr;
}

NodeId Editor::idOf(const GaugeFace* face) const {
    auto it = faceToId.find(const_cast<GaugeFace*>(face));
    return it == faceToId.end() ? 0 : it->second;
}

NodeId Editor::idOf(const Element* element) const {
    auto it = elementToId.find(const_cast<Element*>(element));
    return it == elementToId.end() ? 0 : it->second;
}

Result Editor::getHierarchy() const {
    Result result = OkObject();
    auto& allocator = result.data.GetAllocator();

    rapidjson::Value roots(rapidjson::kArrayType);
    roots.Reserve(static_cast<rapidjson::SizeType>(faces.size()), allocator);

    rapidjson::Value nodesValue(rapidjson::kObjectType);

    for (std::size_t i = 0; i < faces.size(); ++i) {
        const auto& face = faces[i];
        if (!face) continue;

        const NodeId faceId = idOf(face.get());
        roots.PushBack(nodeIdValue(faceId), allocator);

        rapidjson::Value nodeKey(std::to_string(faceId).c_str(), allocator);
        rapidjson::Value nodeValue(rapidjson::kObjectType);

        nodeValue.AddMember("kind", rapidjson::Value("face", allocator), allocator);
        const std::string faceName = i < faceMeta.size() ? faceMeta[i].name : face->typeName();
        nodeValue.AddMember("name", rapidjson::Value(faceName.c_str(), allocator), allocator);

        rapidjson::Value children(rapidjson::kArrayType);
        children.Reserve(static_cast<rapidjson::SizeType>(face->childCount()), allocator);

        for (std::size_t i = 0; i < face->childCount(); ++i) {
            const Element* child = face->childAt(i);
            if (!child) continue;
            children.PushBack(nodeIdValue(idOf(child)), allocator);
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

Result Editor::listElementTypes() const {
    Result result = OkArray();
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

Result Editor::listValueIDs() const {
    Result result = OkArray();
    auto& data = result.data;
    auto& allocator = data.GetAllocator();

    for (const Value& value : Value::list()) {
        const std::string_view id = value.id();
        data.PushBack(
            rapidjson::Value(id.data(), static_cast<rapidjson::SizeType>(id.size()), allocator),
            allocator
        );
    }

    return result;
}

Result Editor::createFace(const std::string& json, FacePlacement where) {
    rapidjson::Document doc = mg::json::parseJson(json);
    if (!doc.IsObject()) return Error("Invalid JSON");

    const std::size_t index = clampIndex(where.index, faces.size());
    const NodeId id = makeId();
    const std::string faceName = "Face " + std::to_string(index + 1);

    auto state = std::make_shared<FaceInsertState>();
    state->index = index;
    state->faceId = id;
    state->meta.name = faceName;

    const bool committed = history.commit({
        "create face",

        // DO
        [this, state, json]() {
            rapidjson::Document doc = mg::json::parseJson(json);

            auto face = std::make_unique<GaugeFace>();
            face->load(doc);

            GaugeFace* raw = face.get();

            faces.insert(
                faces.begin() + static_cast<std::ptrdiff_t>(state->index),
                std::move(face)
            );
            faceMeta.insert(
                faceMeta.begin() + static_cast<std::ptrdiff_t>(state->index),
                state->meta
            );

            registerFaceWithId(state->faceId, raw);

            for (std::size_t i = 0; i < raw->childCount(); ++i) {
                registerSubtree(raw->childAt(i));
            }

            return true;
        },

        // UNDO
        [this, state]() {
            if (state->index >= faces.size()) return false;

            GaugeFace* face = faces[state->index].get();
            unregisterFace(face);

            faces.erase(
                faces.begin() + static_cast<std::ptrdiff_t>(state->index)
            );
            faceMeta.erase(
                faceMeta.begin() + static_cast<std::ptrdiff_t>(state->index)
            );

            return true;
        }
    });

    return committed ? okWithId(id) : Error("Failed to create face");
}

Result Editor::removeFace(NodeId faceId) {
    auto it = std::find_if(faces.begin(), faces.end(),
        [&](const auto& f) { return idOf(f.get()) == faceId; });

    if (it == faces.end()) return Error("Invalid face id");

    const std::size_t index = static_cast<std::size_t>(std::distance(faces.begin(), it));

    auto state = std::make_shared<FaceRemoveState>();
    state->face = it->get();
    state->faceId = faceId;
    state->index = index;
    if (index < faceMeta.size()) {
        state->meta = faceMeta[index];
    }
    state->json = mg::json::toString(state->face->save());
    state->rootIds = snapshotFaceRootIds(*this, *it->get());

    const bool committed = history.commit({
        "remove face",

        // DO
        [this, state]() {
            auto it = std::find_if(faces.begin(), faces.end(),
                [&](const auto& f) { return idOf(f.get()) == state->faceId; });

            if (it == faces.end()) return false;

            unregisterFace(it->get());
            faces.erase(it);
            if (state->index < faceMeta.size()) {
                faceMeta.erase(faceMeta.begin() + static_cast<std::ptrdiff_t>(state->index));
            }
            return true;
        },

        // UNDO
        [this, state]() {
            rapidjson::Document doc = mg::json::parseJson(state->json);
            if (!doc.IsObject()) return false;

            auto face = std::make_unique<GaugeFace>();
            face->load(doc);
            GaugeFace* raw = face.get();

            faces.insert(
                faces.begin() + static_cast<std::ptrdiff_t>(state->index),
                std::move(face)
            );
            faceMeta.insert(
                faceMeta.begin() + static_cast<std::ptrdiff_t>(state->index),
                state->meta
            );

            registerFaceWithId(state->faceId, raw);

            if (state->rootIds.size() != raw->childCount()) return false;

            for (std::size_t i = 0; i < raw->childCount(); ++i) {
                std::size_t next = 0;
                if (!registerSubtreeWithIds(raw->childAt(i), state->rootIds.at(i), next)) return false;
                if (next != state->rootIds.at(i).size()) return false;
            }

            return true;
        }
    });

    return committed ? OkObject() : Error("Failed to remove face");
}

Result Editor::reorderFace(NodeId faceId, std::size_t index) {
    auto it = std::find_if(faces.begin(), faces.end(),
        [&](const auto& f) { return idOf(f.get()) == faceId; });

    if (it == faces.end()) return Error("Invalid face id");

    const std::size_t oldIndex = static_cast<std::size_t>(std::distance(faces.begin(), it));
    const std::size_t newIndex = clampIndex(index, faces.size() - 1);

    if (oldIndex == newIndex) return OkObject();

    auto state = std::make_shared<FaceReorderState>();
    state->faceId = faceId;
    state->from = oldIndex;
    state->to = newIndex;

    const bool committed = history.commit({
        "reorder face",

        // DO
        [this, state]() {
            auto it = faces.begin() + static_cast<std::ptrdiff_t>(state->from);
            auto face = std::move(*it);
            faces.erase(it);
            FaceMeta meta = faceMeta.at(state->from);
            faceMeta.erase(faceMeta.begin() + static_cast<std::ptrdiff_t>(state->from));
            faces.insert(faces.begin() + static_cast<std::ptrdiff_t>(state->to), std::move(face));
            faceMeta.insert(faceMeta.begin() + static_cast<std::ptrdiff_t>(state->to), std::move(meta));
            return true;
        },

        // UNDO
        [this, state]() {
            auto it = faces.begin() + static_cast<std::ptrdiff_t>(state->to);
            auto face = std::move(*it);
            faces.erase(it);
            FaceMeta meta = faceMeta.at(state->to);
            faceMeta.erase(faceMeta.begin() + static_cast<std::ptrdiff_t>(state->to));
            faces.insert(faces.begin() + static_cast<std::ptrdiff_t>(state->from), std::move(face));
            faceMeta.insert(faceMeta.begin() + static_cast<std::ptrdiff_t>(state->from), std::move(meta));
            return true;
        }
    });

    return committed ? OkObject() : Error("Failed to reorder face");
}

Result Editor::createElement(const ElementPlacement& where, const std::string& json) {
    ElementContainerRef parent = getElementContainerById(where.parentId);
    if (!parent.isFace() && !parent.isElement()) return Error("Invalid parent id");

    rapidjson::Document doc = mg::json::parseJson(json);
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

            rapidjson::Document parsed = mg::json::parseJson(state->json);
            OwnedElement element;
            if (!decodeAny(parsed, element)) return false;
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

Result Editor::removeElement(NodeId elementId) {
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
            rapidjson::Document parsed = mg::json::parseJson(state->removed.json);
            OwnedElement restored;
            if (!decodeAny(parsed, restored)) return false;
            Element* raw = restored.get();
            if (!insertIntoContainer(currentParent, std::move(restored), state->index)) return false;
            std::size_t next = 0;
            if (!registerSubtreeWithIds(raw, state->removed.ids, next)) return false;
            return next == state->removed.ids.size();
        }
    });

    return committed ? OkObject() : Error("Failed to remove element");
}

Result Editor::reorderElement(NodeId elementId, std::size_t index) {
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

Result Editor::moveElement(NodeId elementId, const ElementPlacement& where) {
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

Result Editor::replaceElement(NodeId elementId, const std::string& json) {
    Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");

    rapidjson::Document doc = mg::json::parseJson(json);
    if (!doc.IsObject()) return Error("Invalid JSON");

    auto state = std::make_shared<ReplaceState>();
    state->targetId = elementId;
    state->pendingJson = json;
    state->before = snapshotElement(*this, *element);
    state->parentId = element->getParent() ? idOf(element->getParent()) : 0;

    if (element->getParent()) {
        state->index = indexOfChild(*element->getParent(), element);
    } else {
        GaugeFace* ownerFace = element->getOwnerFace();
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

            rapidjson::Document parsed = mg::json::parseJson(state->initialized ? state->after.json : state->pendingJson);
            OwnedElement replacement;
            if (!decodeAny(parsed, replacement)) return false;
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

            rapidjson::Document parsed = mg::json::parseJson(state->before.json);
            OwnedElement restored;
            if (!decodeAny(parsed, restored)) return false;
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

Result Editor::setProperty(NodeId id, const std::string& path, const std::string& json) {
    ::mg::PropertyObject* object = getObjectById(id);
    if (!object) return Error("Invalid id");

    rapidjson::Document value = mg::json::parseJson(json);
    if (value.HasParseError()) return Error("Invalid JSON");

    ::mg::PropertyObject* owner = nullptr;
    const ::mg::Property* property = nullptr;
    if (!object->resolvePath(path, owner, property) || !owner || !property) {
        return Error("Invalid property path");
    }

    rapidjson::Document beforeDoc;
    beforeDoc.SetObject();
    if (!owner->getProperty(property->key, beforeDoc, beforeDoc.GetAllocator())) {
        return Error("Failed to read current property value");
    }

    const std::string beforeJson = mg::json::toString(beforeDoc);
    auto state = std::make_shared<PropertySetState>();
    state->nodeId = id;
    state->path = path;
    state->beforeJson = beforeJson;
    state->afterJson = json;

    const bool committed = history.commit({
        "set property",
        [this, state]() {
    ::mg::PropertyObject* currentObject = getObjectById(state->nodeId);
            if (!currentObject) return false;
    ::mg::PropertyObject* currentOwner = nullptr;
            const ::mg::Property* currentProperty = nullptr;
            if (!currentObject->resolvePath(state->path, currentOwner, currentProperty) || !currentOwner || !currentProperty) return false;
            rapidjson::Document parsed = mg::json::parseJson(state->afterJson);
            return currentOwner->setProperty(currentProperty->key, parsed);
        },
        [this, state]() {
    ::mg::PropertyObject* currentObject = getObjectById(state->nodeId);
            if (!currentObject) return false;
    ::mg::PropertyObject* currentOwner = nullptr;
            const ::mg::Property* currentProperty = nullptr;
            if (!currentObject->resolvePath(state->path, currentOwner, currentProperty) || !currentOwner || !currentProperty) return false;
            rapidjson::Document parsed = mg::json::parseJson(state->beforeJson);
            return currentOwner->setProperty(currentProperty->key, parsed);
        }
    });

    return committed ? OkObject() : Error("Failed to set property");
}

Result Editor::getProperty(NodeId id, const std::string& path) const {
    const NodeRef* node = getNode(id);
    if (!node) return Error("Invalid id");
    if (path.empty()) return Error("Property path is required");

    const ::mg::PropertyObject* object = node->kind == NodeKind::Face ? static_cast<const ::mg::PropertyObject*>(node->face) : static_cast<const ::mg::PropertyObject*>(node->element);

    const ::mg::PropertyObject* owner = nullptr;
    const ::mg::Property* property = nullptr;
    if (!object->resolvePath(path, owner, property) || !owner || !property) return Error("Invalid property path");

    Result result = OkObject();
    rapidjson::Value value;
    if (!owner->getProperty(property->key, value, result.data.GetAllocator())) return Error("Failed to read property");

    result.data.AddMember("id", nodeIdValue(id), result.data.GetAllocator());
    result.data.AddMember("path", rapidjson::Value(path.c_str(), result.data.GetAllocator()), result.data.GetAllocator());
    result.data.AddMember("value", std::move(value), result.data.GetAllocator());
    return result;
}

Result Editor::getPropertiesMeta(NodeId id, const std::string& path) const {
    const NodeRef* node = getNode(id);
    if (!node) return Error("Invalid id");

    const ::mg::PropertyObject* object = node->kind == NodeKind::Face ? static_cast<const ::mg::PropertyObject*>(node->face) : static_cast<const ::mg::PropertyObject*>(node->element);

    Result result = OkObject();
    auto& allocator = result.data.GetAllocator();
    result.data.AddMember("id", nodeIdValue(id), allocator);

    if (path.empty()) {
        rapidjson::Value meta = object->getPropertiesMeta(allocator);
        result.data.AddMember("meta", std::move(meta), allocator);
        return result;
    }

    const ::mg::PropertyObject* owner = nullptr;
    const ::mg::Property* property = nullptr;
    if (!object->resolvePath(path, owner, property) || !owner || !property) return Error("Invalid property path");

    rapidjson::Value meta = owner->getPropertyMeta(*property, allocator);
    result.data.AddMember("path", rapidjson::Value(path.c_str(), allocator), allocator);
    result.data.AddMember("meta", std::move(meta), allocator);
    return result;
}

Result Editor::getHistory() const {
    Result result = OkArray();
    auto& allocator = result.data.GetAllocator();

    for (const auto& name : history.names()) {
        result.data.PushBack(rapidjson::Value(name.c_str(), allocator), allocator);
    }

    return result;
}

}
