#include <multigauge/editor/Editor.h>

#include <algorithm>
#include <cstdint>
#include <memory>

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
    json::Document doc = json::object();
    json::Writer writer = doc.writer();
    if (!element.saveProperties(writer)) return snapshot;
    snapshot.json = doc.toString();
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

Result okWithId(NodeId id) {
    Result result = OkObject();
    json::Writer writer = result.data.writer();
    if (!writer.writeObject([&](json::ObjectWriter& object) { return object.write("id", static_cast<std::uint64_t>(id)); })) return Error("Failed to build result");
    return result;
}

Result okWithIds(NodeId id, NodeId parentId) {
    Result result = OkObject();
    json::Writer writer = result.data.writer();
    if (!writer.writeObject([&](json::ObjectWriter& object) { return object.write("id", static_cast<std::uint64_t>(id)) && object.write("parentId", static_cast<std::uint64_t>(parentId)); })) return Error("Failed to build result");
    return result;
}

bool validatePackageShape(json::Reader value) {
    (void)value;
    return true;
}

bool appendElementHierarchyNode(const Editor& editor, const Element& element, json::ObjectWriter& nodes) {
    const NodeId id = editor.idOf(&element);
    if (!nodes.writeObject(std::to_string(id), [&](json::ObjectWriter& node) { return node.write("kind", "element") && node.write("type", element.typeName()) && node.write("name", element.typeName()) && node.writeArray("children", [&](json::ArrayWriter& children) { for (std::size_t i = 0; i < element.childCount(); ++i) { const Element* child = element.childAt(i); if (child && !children.write(static_cast<std::uint64_t>(editor.idOf(child)))) return false; } return true; }); })) return false;

    for (std::size_t i = 0; i < element.childCount(); ++i) {
        const Element* child = element.childAt(i);
        if (!child) continue;
        if (!appendElementHierarchyNode(editor, *child, nodes)) return false;
    }
    return true;
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
    json::Document doc = mg::json::parse(json);
    if (!doc.valid() || !doc.root().isObject()) return false;

    std::string name;
    std::string author;
    std::string description;
    if (!mg::json::getStringMember(doc.root(), "name", name) || !mg::json::getStringMember(doc.root(), "author", author) || !mg::json::getStringMember(doc.root(), "description", description)) {
        return false;
    }

    const json::Reader faces = mg::json::getArrayMember(doc.root(), "faces");
    if (!faces.valid()) return false;

    clear();
    packageName = std::move(name);
    packageAuthor = std::move(author);
    packageDescription = std::move(description);

    for (std::size_t index = 0; index < faces.size(); ++index) {
        const json::Reader faceEntry = faces.element(index);
        if (!faceEntry.isObject()) return false;

        std::string faceName;
        if (!mg::json::getStringMember(faceEntry, "name", faceName)) {
            return false;
        }

        const json::Reader faceJson = mg::json::getObjectMember(faceEntry, "face");
        if (!faceJson.valid()) return false;

        auto face = std::make_unique<GaugeFace>();
        if (!face->load(faceJson)) return false;

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
    json::Document doc = json::object();
    if (!doc.writer().writeObject([&](json::ObjectWriter& object) { return object.write("name", packageName) && object.write("author", packageAuthor) && object.write("description", packageDescription) && object.writeArray("faces", [&](json::ArrayWriter& facesOut) { for (std::size_t i = 0; i < this->faces.size(); ++i) {
        const auto& face = this->faces[i];
        if (!face) continue;

        const FaceMeta meta = i < faceMeta.size() ? faceMeta[i] : FaceMeta{ "Face" };
        if (!facesOut.writeObject([&](json::ObjectWriter& entry) { return entry.write("name", meta.name) && entry.writeObject("face", [&](json::ObjectWriter& faceValue) { json::Writer& faceWriter = faceValue.writer(); return face->save(faceWriter); }); })) return false;
    }
    return true; }); })) return {};
    return doc.toString();
}

Result Editor::serializeFace(NodeId faceId) const {
    const GaugeFace* face = getFaceById(faceId);
    if (!face) return Error("Invalid face id");
    json::Document document = json::object();
    json::Writer writer = document.writer();
    if (!face->save(writer)) return Error("Failed to serialize face");
    std::string json = document.toString();
    Result result = OkObject();
    result.data.writer().writeObject([&](json::ObjectWriter& object) { return object.write("json", json); });
    return result;
}

Result Editor::serializeElement(NodeId elementId) const {
    const Element* element = getElementById(elementId);
    if (!element) return Error("Invalid element id");
    json::Document doc = json::object();
    json::Writer writer = doc.writer();
    if (!element->saveProperties(writer)) return Error("Failed to serialize element");
    std::string json = doc.toString();
    Result result = OkObject();
    result.data.writer().writeObject([&](json::ObjectWriter& object) { return object.write("json", json); });
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
    if (!result.data.writer().writeObject([&](json::ObjectWriter& output) { return output.writeArray("roots", [&](json::ArrayWriter& roots) { for (const auto& face : faces) if (face && !roots.write(static_cast<std::uint64_t>(idOf(face.get())))) return false; return true; }) && output.writeObject("nodes", [&](json::ObjectWriter& nodesValue) { for (std::size_t i = 0; i < faces.size(); ++i) {
        const auto& face = faces[i];
        if (!face) continue;

        const NodeId faceId = idOf(face.get());
        const std::string faceName = i < faceMeta.size() ? faceMeta[i].name : face->typeName();
        if (!nodesValue.writeObject(std::to_string(faceId), [&](json::ObjectWriter& node) { return node.write("kind", "face") && node.write("name", faceName) && node.writeArray("children", [&](json::ArrayWriter& children) { for (std::size_t childIndex = 0; childIndex < face->childCount(); ++childIndex) { const Element* child = face->childAt(childIndex); if (child && !children.write(static_cast<std::uint64_t>(idOf(child)))) return false; } return true; }); })) return false;

        for (std::size_t i = 0; i < face->childCount(); ++i) {
            const Element* child = face->childAt(i);
            if (!child) continue;
            if (!appendElementHierarchyNode(*this, *child, nodesValue)) return false;
        }
    }
    return true; }); })) return Error("Failed to build hierarchy");

    return result;
}

Result Editor::listElementTypes() const {
    Result result = OkArray();
    if (!result.data.writer().writeArray([&](json::ArrayWriter& data) { for (const auto& descriptor : Element::registry()) if (!data.writeObject([&](json::ObjectWriter& entry) { return entry.write("name", descriptor.name ? descriptor.name : "") && entry.write("type", descriptor.id ? descriptor.id : ""); })) return false; return true; })) return Error("Failed to list element types");

    return result;
}

Result Editor::listValueIDs() const {
    Result result = OkArray();
    if (!result.data.writer().writeArray([&](json::ArrayWriter& data) { for (const Value& value : Value::list()) if (!data.write(value.id())) return false; return true; })) return Error("Failed to list values");

    return result;
}

Result Editor::createFace(const std::string& json, FacePlacement where) {
    json::Document doc = mg::json::parse(json);
    if (!doc.valid() || !doc.root().isObject()) return Error("Invalid JSON");

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
            json::Document doc = mg::json::parse(json);
            if (!doc.valid() || !doc.root().isObject()) return false;

            auto face = std::make_unique<GaugeFace>();
            if (!face->load(doc.root())) return false;

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
    { json::Document document = json::object(); json::Writer writer = document.writer(); if (!state->face->save(writer)) return Error("Failed to serialize face"); state->json = document.toString(); }
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
            json::Document doc = mg::json::parse(state->json);
            if (!doc.valid() || !doc.root().isObject()) return false;

            auto face = std::make_unique<GaugeFace>();
            if (!face->load(doc.root())) return false;
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

    json::Document doc = mg::json::parse(json);
    if (!doc.valid() || !doc.root().isObject()) return Error("Invalid JSON");

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

            json::Document parsed = mg::json::parse(state->json);
            OwnedElement element;
            if (!parsed.valid() || !decodeAny(parsed.root(), element)) return false;
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
            json::Document parsed = mg::json::parse(state->removed.json);
            OwnedElement restored;
            if (!parsed.valid() || !decodeAny(parsed.root(), restored)) return false;
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

    json::Document doc = mg::json::parse(json);
    if (!doc.valid() || !doc.root().isObject()) return Error("Invalid JSON");

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

            json::Document parsed = mg::json::parse(state->initialized ? state->after.json : state->pendingJson);
            OwnedElement replacement;
            if (!parsed.valid() || !decodeAny(parsed.root(), replacement)) return false;
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

            json::Document parsed = mg::json::parse(state->before.json);
            OwnedElement restored;
            if (!parsed.valid() || !decodeAny(parsed.root(), restored)) return false;
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

    json::Document value = mg::json::parse(json);
    if (!value.valid()) return Error("Invalid JSON");

    ::mg::PropertyObject* owner = nullptr;
    const ::mg::Property* property = nullptr;
    if (!object->resolvePath(path, owner, property) || !owner || !property) {
        return Error("Invalid property path");
    }

    json::Document beforeDoc = json::object();
    json::Writer beforeWriter = beforeDoc.writer();
    if (!owner->getProperty(property->key, beforeWriter)) {
        return Error("Failed to read current property value");
    }

    const std::string beforeJson = beforeDoc.toString();
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
            json::Document parsed = mg::json::parse(state->afterJson);
            if (!parsed.valid() || !currentOwner->setProperty(currentProperty->key, parsed.root())) return false;
            invalidateLayoutForProperty(state->nodeId, state->path);
            return true;
        },
        [this, state]() {
    ::mg::PropertyObject* currentObject = getObjectById(state->nodeId);
            if (!currentObject) return false;
    ::mg::PropertyObject* currentOwner = nullptr;
            const ::mg::Property* currentProperty = nullptr;
            if (!currentObject->resolvePath(state->path, currentOwner, currentProperty) || !currentOwner || !currentProperty) return false;
            json::Document parsed = mg::json::parse(state->beforeJson);
            if (!parsed.valid() || !currentOwner->setProperty(currentProperty->key, parsed.root())) return false;
            invalidateLayoutForProperty(state->nodeId, state->path);
            return true;
        }
    });

    return committed ? OkObject() : Error("Failed to set property");
}

void Editor::invalidateLayoutForProperty(NodeId id, const std::string& path) {
    const auto inGroup = [&](std::string_view group) {
        return path == group || (path.size() > group.size() && path.compare(0, group.size(), group) == 0 && path[group.size()] == '.');
    };

    if (Element* element = getElementById(id)) {
        if (inGroup("style")) element->markLayoutDirty();
    } else if (GaugeFace* face = getFaceById(id)) {
        if (inGroup("layout")) face->markLayoutDirty();
    }
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
    if (!result.data.writer().writeObject([&](json::ObjectWriter& output) { return output.write("id", static_cast<std::uint64_t>(id)) && output.write("path", path) && output.writeValue("value", [&](json::Writer& writer) { return owner->getProperty(property->key, writer); }); })) return Error("Failed to read property");
    return result;
}

Result Editor::getPropertiesMeta(NodeId id, const std::string& path) const {
    const NodeRef* node = getNode(id);
    if (!node) return Error("Invalid id");

    const ::mg::PropertyObject* object = node->kind == NodeKind::Face ? static_cast<const ::mg::PropertyObject*>(node->face) : static_cast<const ::mg::PropertyObject*>(node->element);

    Result result = OkObject();

    if (path.empty()) {
        if (!result.data.writer().writeObject([&](json::ObjectWriter& output) { return output.write("id", static_cast<std::uint64_t>(id)) && output.writeValue("meta", [&](json::Writer& writer) { return object->writePropertiesMeta(writer); }); })) return Error("Failed to write property metadata");
        return result;
    }

    const ::mg::PropertyObject* owner = nullptr;
    const ::mg::Property* property = nullptr;
    if (!object->resolvePath(path, owner, property) || !owner || !property) return Error("Invalid property path");

    if (!result.data.writer().writeObject([&](json::ObjectWriter& output) { return output.write("id", static_cast<std::uint64_t>(id)) && output.write("path", path) && output.writeValue("meta", [&](json::Writer& writer) { return owner->writePropertyMeta(writer, *property); }); })) return Error("Failed to write property metadata");
    return result;
}

Result Editor::getHistory() const {
    Result result = OkArray();
    if (!result.data.writer().writeArray([&](json::ArrayWriter& output) { for (const auto& name : history.names()) if (!output.write(name)) return false; return true; })) return Error("Failed to write history");

    return result;
}

}
