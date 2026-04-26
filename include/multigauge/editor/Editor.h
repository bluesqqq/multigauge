#pragma once

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include <multigauge/editor/Clipboard.h>
#include <multigauge/editor/EditorResult.h>
#include <multigauge/editor/History.h>
#include <multigauge/gauge/GaugeFace.h>

class Editor {
public:
    using Id = uint32_t;
    static constexpr std::size_t Append = static_cast<std::size_t>(-1);

    enum class NodeKind {
        Face,
        Element
    };

    struct FacePlacement {
        std::size_t index = Append;
    };

    struct RootPlacement {
        Id faceId = 0;
        std::size_t index = Append;
    };

    struct ChildPlacement {
        Id parentElementId = 0;
        std::size_t index = Append;
    };

private:
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

    std::vector<GaugeFace*> faces;
    std::vector<std::unique_ptr<GaugeFace>> ownedFaces;

    std::unordered_map<Id, NodeRef> nodes;
    std::unordered_map<GaugeFace*, Id> faceToId;
    std::unordered_map<Element*, Id> elementToId;

    Id nextId = 1;
    History history;
    ClipboardState clipboard;

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
    PropertyObject* getObjectById(Id id);
    const PropertyObject* getObjectById(Id id) const;

    static std::size_t clampIndex(std::size_t index, std::size_t size) {
        return (index == Append || index > size) ? size : index;
    }

public:
    Editor() = default;

    //----------[ DOCUMENT ]----------//

    void clear();
    void loadDocument(const std::string& json);
    std::string saveDocument() const;

    //----------[ QUERIES ]----------//

    bool hasNode(Id id) const;
    bool isFace(Id id) const;
    bool isElement(Id id) const;

    Id idOf(const GaugeFace* face) const;
    Id idOf(const Element* element) const;

    std::size_t faceCount() const { return faces.size(); }
    GaugeFace* faceAt(std::size_t index) { return faces.at(index); }
    const GaugeFace* faceAt(std::size_t index) const { return faces.at(index); }

    EditorResult listFaces() const;
    EditorResult listRoots(Id faceId) const;
    EditorResult listChildren(Id elementId) const;
    EditorResult describeNode(Id id) const;

    //----------[ FACE LIST ]----------//

    EditorResult insertFace(GaugeFace& face, FacePlacement where = {});
    EditorResult removeFace(Id faceId);
    EditorResult reorderFace(Id faceId, std::size_t index);

    //----------[ ROOT ELEMENTS ]----------//

    EditorResult createRoot(const RootPlacement& where, const std::string& json);
    EditorResult removeRoot(Id elementId);
    EditorResult reorderRoot(Id elementId, std::size_t index);
    EditorResult moveRootToFace(Id elementId, const RootPlacement& where);

    //----------[ CHILD ELEMENTS ]----------//

    EditorResult createChild(const ChildPlacement& where, const std::string& json);
    EditorResult removeElement(Id elementId);
    EditorResult reorderChild(Id elementId, std::size_t index);
    EditorResult moveElementToParent(Id elementId, const ChildPlacement& where);
    EditorResult replaceElementFromJson(Id elementId, const std::string& json);

    //----------[ PROPERTIES ]----------//

    EditorResult setProperty(Id id, const std::string& path, const std::string& json);
    EditorResult getProperty(Id id, const std::string& path) const;
    EditorResult getPropertiesMeta(Id id, const std::string& path = "") const;

    //----------[ CLIPBOARD ]----------//

    ClipboardSummary getClipboardSummary() const { return { clipboard.kind }; }
    void clearClipboard() { clipboard.clear(); }

    EditorResult copyFace(Id faceId);
    EditorResult cutFace(Id faceId);
    EditorResult pasteFace(FacePlacement where = {});

    EditorResult copyElement(Id elementId);
    EditorResult cutRoot(Id elementId);
    EditorResult cutElement(Id elementId);
    EditorResult pasteRoot(const RootPlacement& where);
    EditorResult pasteChild(const ChildPlacement& where);
    EditorResult pasteToReplaceElement(Id elementId);

    //----------[ HISTORY ]----------//

    bool canUndo() const { return history.canUndo(); }
    bool canRedo() const { return history.canRedo(); }
    bool undo() { return history.undo(); }
    bool redo() { return history.redo(); }
    EditorResult getHistory() const;
};
