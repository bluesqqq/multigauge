#pragma once

#include <cstddef>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <multigauge/editor/Clipboard.h>
#include <multigauge/editor/Types.h>
#include <multigauge/editor/Result.h>
#include <multigauge/editor/History.h>
#include <multigauge/gauge/GaugeFace.h>

namespace mg::editor {

using ::mg::Result;
using ::mg::gauge::Element;
using ::mg::gauge::GaugeFace;
using ::mg::gauge::OwnedElement;

class Editor {
    public:
        /// Appends at the end of a sibling list when used as an index.
        static constexpr std::size_t Append = static_cast<std::size_t>(-1);

        enum class NodeKind {
            Face,
            Element
        };

    private:
        struct ElementContainerRef {
            GaugeFace* face = nullptr;
            Element* element = nullptr;

            bool isFace() const { return face != nullptr; }
            bool isElement() const { return element != nullptr; }
        };

        struct NodeRef {
            NodeKind kind = NodeKind::Face;
            union {
                GaugeFace* face;
                Element* element;
            };

            NodeRef() : face(nullptr) {}
            explicit NodeRef(GaugeFace* value) : kind(NodeKind::Face), face(value) {}
            explicit NodeRef(Element* value) : kind(NodeKind::Element), element(value) {}
        };

        std::vector<std::unique_ptr<GaugeFace>> faces;

        std::unordered_map<NodeId, NodeRef> nodes;
        std::unordered_map<GaugeFace*, NodeId> faceToId;
        std::unordered_map<Element*, NodeId> elementToId;

        NodeId nextId = 1;
        History history;
    private:
        //----------[ INTERNAL HELPERS ]----------//

        NodeId makeId() { return nextId++; }

        NodeRef* getNode(NodeId id);
        const NodeRef* getNode(NodeId id) const;

        NodeId registerFace(GaugeFace* face);
        NodeId registerFaceWithId(NodeId id, GaugeFace* face);
        NodeId registerElement(Element* element);
        NodeId registerElementWithId(NodeId id, Element* element);
        void registerSubtree(Element* element);
        bool registerSubtreeWithIds(Element* element, const std::vector<NodeId>& ids, std::size_t& nextIndex);
        void unregisterFace(GaugeFace* face);
        void unregisterElementRecursive(Element* element);

        GaugeFace* getFaceById(NodeId id);
        const GaugeFace* getFaceById(NodeId id) const;

        Element* getElementById(NodeId id);
        const Element* getElementById(NodeId id) const;
        ::mg::PropertyObject* getObjectById(NodeId id);
        const ::mg::PropertyObject* getObjectById(NodeId id) const;
        ElementContainerRef getElementContainerById(NodeId id);
        ElementContainerRef getElementContainerOf(Element* element);
        NodeId idOfContainer(const ElementContainerRef& container) const;
        bool insertIntoContainer(const ElementContainerRef& container, OwnedElement child, std::size_t index);
        OwnedElement removeFromContainer(const ElementContainerRef& container, Element* child);
        std::size_t childCountOf(const ElementContainerRef& container) const {
            if (container.face) return container.face->childCount();
            if (container.element) return container.element->childCount();
            return 0;
        }
        std::size_t indexInContainer(const ElementContainerRef& container, const Element* child) const {
            if (container.face) {
                for (std::size_t i = 0; i < container.face->childCount(); ++i) {
                    if (container.face->childAt(i) == child) return i;
                }
                return Append;
            }
            if (container.element) {
                for (std::size_t i = 0; i < container.element->childCount(); ++i) {
                    if (container.element->childAt(i) == child) return i;
                }
                return Append;
            }
            return Append;
        }

        static std::size_t clampIndex(std::size_t index, std::size_t size) {
            return (index == Append || index > size) ? size : index;
        }

    public:
        Editor() = default;
        Editor(const Editor&) = delete;
        Editor& operator=(const Editor&) = delete;
        Editor(Editor&&) noexcept = default;
        Editor& operator=(Editor&&) noexcept = default;

        //----------[ DOCUMENT ]----------//

        /// Clears the loaded document, history, and node IDs.
        void clear();
        
        /// Loads faces and elements from a document JSON array.
        bool loadDocument(const std::string& json);
        
        /// Saves the current document as a JSON array.
        std::string saveDocument() const;

        /// Serializes a single face as JSON.
        /// @return `{ "json": string }`.
        Result serializeFace(NodeId faceId) const;

        /// Serializes a single element as JSON.
        /// @return `{ "json": string }`.
        Result serializeElement(NodeId elementId) const;

        //----------[ QUERIES ]----------//

        /// Returns whether a face or element with `id` exists.
        bool hasNode(NodeId id) const;

        /// Returns whether `id` refers to a face.
        bool isFace(NodeId id) const;

        /// Returns whether `id` refers to an element.
        bool isElement(NodeId id) const;

        /// Returns the editor ID for `face`.
        /// @return Face ID, or `0` if `face` is not registered.
        NodeId idOf(const GaugeFace* face) const;

        /// Returns the editor ID for `element`.
        /// @return Element ID, or `0` if `element` is not registered.
        NodeId idOf(const Element* element) const;

        std::size_t faceCount() const { return faces.size(); }
        GaugeFace* faceAt(std::size_t index) { return faces.at(index).get(); }
        const GaugeFace* faceAt(std::size_t index) const { return faces.at(index).get(); }

        /// Returns the face and element tree as a flat hierarchy payload.
        /// @return `{ "roots": [uint...], "nodes": { "id": { "kind": string, "name": string, "children": [uint...] }, ... } }`
        /// where element nodes also include `"type": string`.
        Result getHierarchy() const;

        /// Returns the available element type descriptors.
        Result listElementTypes() const;

        /// Returns the available value IDs.
        Result listValueIDs() const;

        //----------[ GAUGE FACES ]----------//

        /// Creates a new face in the face list.
        /// @note `where.index` appends when set to `Append` or greater than the face count.
        /// @return `{ "id": uint }` for the inserted face.
        Result createFace(const std::string& json, FacePlacement where = FacePlacement{});

        /// Removes a face from the face list.
        Result removeFace(NodeId faceId);

        /// Reorders a face within the face list.
        /// @note `index` is zero-based and clamps to the valid face range.
        Result reorderFace(NodeId faceId, std::size_t index);

        //----------[ ELEMENTS ]----------//

        /// Creates an element under a face or element parent from JSON.
        /// @note `where.parentId` may refer to either a face or an element.
        /// @note `where.index` appends when set to `Append` or greater than the parent child count.
        /// @return `{ "id": uint, "parentId": uint }` for the inserted element.
        Result createElement(const ElementPlacement& where, const std::string& json);

        /// Removes an element from its current parent.
        Result removeElement(NodeId elementId);

        /// Reorders an element within its current parent.
        /// @note `index` is zero-based and clamps to the valid sibling range.
        Result reorderElement(NodeId elementId, std::size_t index);

        /// Moves an element to a new face or element parent.
        /// @note `where.parentId` may refer to either a face or an element.
        /// @note `where.index` appends when set to `Append` or greater than the destination child count.
        /// @return `{ "id": uint, "parentId": uint }` for the moved element.
        Result moveElement(NodeId elementId, const ElementPlacement& where);

        /// Replaces an element with a new element loaded from JSON.
        /// @return `{ "id": uint }` for the replaced element.
        Result replaceElement(NodeId elementId, const std::string& json);

        //----------[ PROPERTIES ]----------//

        /// Sets a property on a face or element from JSON.
        /// @param path Dotted property path such as `"style.margin.left"`.
        Result setProperty(NodeId id, const std::string& path, const std::string& json);

        /// Gets a property from a face or element.
        /// @param path Dotted property path such as `"style.margin.left"`.
        /// @return `{ "id": uint, "path": string, "value": any }`.
        Result getProperty(NodeId id, const std::string& path) const;

        /// Gets property metadata from a face or element.
        /// @param path Dotted property path, or empty to describe the whole object.
        /// @return `{ "id": uint, "meta": object }` for an empty `path`, or
        /// `{ "id": uint, "path": string, "meta": object }` for a resolved property.
        Result getPropertiesMeta(NodeId id, const std::string& path = "") const;

        //----------[ HISTORY ]----------//

        bool canUndo() const { return history.canUndo(); }
        bool canRedo() const { return history.canRedo(); }

        /// Undoes the most recent committed edit.
        bool undo() { return history.undo(); }

        /// Redoes the next committed edit.
        bool redo() { return history.redo(); }

        /// Jumps the history cursor to `index`.
        bool jumpTo(std::size_t index) { return history.jumpTo(index); }

        std::size_t historyIndex() { return history.headIndex(); }

        /// Returns the history command names in order.
        /// @return `[ string, ... ]`.
        Result getHistory() const;
};

}
