#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <multigauge/editor/Clipboard.h>
#include <multigauge/editor/Result.h>
#include <multigauge/editor/History.h>
#include <multigauge/gauge/GaugeFace.h>

namespace mg::editor {

using ::mg::gauge::Element;
using ::mg::gauge::GaugeFace;
using ::mg::gauge::OwnedElement;

class Editor {
    public:
        /// Identifies a face or element within the editor.
        using Id = uint32_t;
        /// Appends at the end of a sibling list when used as an index.
        static constexpr std::size_t Append = static_cast<std::size_t>(-1);

        enum class NodeKind {
            Face,
            Element
        };

        struct FacePlacement {
            std::size_t index;

            explicit FacePlacement(std::size_t i = Append) : index(i) {}
        };

        struct ElementPlacement {
            Id parentId;
            std::size_t index;

            ElementPlacement(Id p = 0, std::size_t i = Append) : parentId(p), index(i) {}
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

        std::unordered_map<Id, NodeRef> nodes;
        std::unordered_map<GaugeFace*, Id> faceToId;
        std::unordered_map<Element*, Id> elementToId;

        Id nextId = 1;
        History history;
    private:
        //----------[ INTERNAL HELPERS ]----------//

        Id makeId() { return nextId++; }

        NodeRef* getNode(Id id);
        const NodeRef* getNode(Id id) const;

        Id registerFace(GaugeFace* face);
        Id registerFaceWithId(Id id, GaugeFace* face);
        Id registerElement(Element* element);
        Id registerElementWithId(Id id, Element* element);
        void registerSubtree(Element* element);
        bool registerSubtreeWithIds(Element* element, const std::vector<Id>& ids, std::size_t& nextIndex);
        void unregisterFace(GaugeFace* face);
        void unregisterElementRecursive(Element* element);

        GaugeFace* getFaceById(Id id);
        const GaugeFace* getFaceById(Id id) const;

        Element* getElementById(Id id);
        const Element* getElementById(Id id) const;
        ::mg::PropertyObject* getObjectById(Id id);
        const ::mg::PropertyObject* getObjectById(Id id) const;
        ElementContainerRef getElementContainerById(Id id);
        ElementContainerRef getElementContainerOf(Element* element);
        Id idOfContainer(const ElementContainerRef& container) const;
        std::size_t childCountOf(const ElementContainerRef& container) const;
        bool insertIntoContainer(const ElementContainerRef& container, OwnedElement child, std::size_t index);
        OwnedElement removeFromContainer(const ElementContainerRef& container, Element* child);
        std::size_t indexInContainer(const ElementContainerRef& container, const Element* child) const;

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
        void loadDocument(const std::string& json);
        /// Saves the current document as a JSON array.
        std::string saveDocument() const;

        //----------[ QUERIES ]----------//

        /// Returns whether a face or element with `id` exists.
        bool hasNode(Id id) const;

        /// Returns whether `id` refers to a face.
        bool isFace(Id id) const;

        /// Returns whether `id` refers to an element.
        bool isElement(Id id) const;

        /// Returns the editor ID for `face`.
        /// @return Face ID, or `0` if `face` is not registered.
        Id idOf(const GaugeFace* face) const;

        /// Returns the editor ID for `element`.
        /// @return Element ID, or `0` if `element` is not registered.
        Id idOf(const Element* element) const;

        std::size_t faceCount() const { return faces.size(); }
        GaugeFace* faceAt(std::size_t index) { return faces.at(index).get(); }
        const GaugeFace* faceAt(std::size_t index) const { return faces.at(index).get(); }

        /// Returns the face and element tree as a flat hierarchy payload.
        /// @return `{ "roots": [uint...], "nodes": { "id": { "kind": string, "name": string, "children": [uint...] }, ... } }`
        /// where element nodes also include `"type": string`.
        Result getHierarchy() const;

        /// Lists registered element types for editor UI creation menus.
        /// @return `[ { "name": string, "type": string }, ... ]`.
        Result listElementTypes() const;

        /// Lists known value IDs for editor UI binding menus.
        /// @return `[ string, ... ]`.
        Result listValueIDs() const;

        //----------[ GAUGE FACES ]----------//

        /// Creates a new face in the face list.
        /// @note `where.index` appends when set to `Append` or greater than the face count.
        /// @return `{ "id": uint }` for the inserted face.
        Result createFace(const std::string& json, FacePlacement where = FacePlacement{});

        /// Removes a face from the face list.
        Result removeFace(Id faceId);

        /// Reorders a face within the face list.
        /// @note `index` is zero-based and clamps to the valid face range.
        Result reorderFace(Id faceId, std::size_t index);

        //----------[ ELEMENTS ]----------//

        /// Creates an element under a face or element parent from JSON.
        /// @note `where.parentId` may refer to either a face or an element.
        /// @note `where.index` appends when set to `Append` or greater than the parent child count.
        /// @return `{ "id": uint, "parentId": uint }` for the inserted element.
        Result createElement(const ElementPlacement& where, const std::string& json);

        /// Removes an element from its current parent.
        Result removeElement(Id elementId);

        /// Reorders an element within its current parent.
        /// @note `index` is zero-based and clamps to the valid sibling range.
        Result reorderElement(Id elementId, std::size_t index);

        /// Moves an element to a new face or element parent.
        /// @note `where.parentId` may refer to either a face or an element.
        /// @note `where.index` appends when set to `Append` or greater than the destination child count.
        /// @return `{ "id": uint, "parentId": uint }` for the moved element.
        Result moveElement(Id elementId, const ElementPlacement& where);

        /// Replaces an element with a new element loaded from JSON.
        /// @return `{ "id": uint }` for the replaced element.
        Result replaceElementFromJson(Id elementId, const std::string& json);

        //----------[ PROPERTIES ]----------//

        /// Sets a property on a face or element from JSON.
        /// @param path Dotted property path such as `"style.margin.left"`.
        Result setProperty(Id id, const std::string& path, const std::string& json);

        /// Gets a property from a face or element.
        /// @param path Dotted property path such as `"style.margin.left"`.
        /// @return `{ "id": uint, "path": string, "value": any }`.
        Result getProperty(Id id, const std::string& path) const;

        /// Gets property metadata from a face or element.
        /// @param path Dotted property path, or empty to describe the whole object.
        /// @return `{ "id": uint, "meta": object }` for an empty `path`, or
        /// `{ "id": uint, "path": string, "meta": object }` for a resolved property.
        Result getPropertiesMeta(Id id, const std::string& path = "") const;

        //----------[ CLIPBOARD ]----------//

        ClipboardSummary getClipboardSummary() const;
        void clearClipboard();

        /// Copies a face into the editor clipboard.
        Result copyFace(Id faceId);

        /// Copies a face into the editor clipboard and removes it.
        Result cutFace(Id faceId);

        /// Pastes a face from the editor clipboard.
        /// @note `where.index` appends when set to `Append` or greater than the face count.
        /// @return `{ "id": uint }` for the inserted face.
        Result pasteFace(FacePlacement where = FacePlacement{});

        /// Copies an element into the editor clipboard.
        Result copyElement(Id elementId);

        /// Copies an element into the editor clipboard and removes it.
        Result cutElement(Id elementId);

        /// Pastes an element from the editor clipboard under a face or element parent.
        /// @note `where.parentId` may refer to either a face or an element.
        /// @return `{ "id": uint, "parentId": uint }` for the inserted element.
        Result pasteElement(const ElementPlacement& where);

        /// Replaces an element with the current clipboard element payload.
        /// @return `{ "id": uint }` for the replaced element.
        Result pasteToReplaceElement(Id elementId);

        //----------[ HISTORY ]----------//

        bool canUndo() const { return history.canUndo(); }
        bool canRedo() const { return history.canRedo(); }

        /// Undoes the most recent committed edit.
        bool undo() { return history.undo(); }

        /// Redoes the next committed edit.
        bool redo() { return history.redo(); }

        /// Returns undo and redo availability for editor UI state.
        /// @return `{ "canUndo": bool, "canRedo": bool }`.
        Result getHistory() const;
};

}
