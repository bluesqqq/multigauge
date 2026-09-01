#pragma once

#include <cstddef>
#include <string>

#include <multigauge/container/HandlePool.h>
#include <multigauge/editor/Clipboard.h>
#include <multigauge/editor/Editor.h>

namespace mg::editor {

/// Owns editor instances for one Runtime.
class Manager {
public:
    [[nodiscard]] EditorId create() { return editors_.emplace(); }
    [[nodiscard]] bool destroy(EditorId id) { return editors_.remove(id); }
    [[nodiscard]] bool exists(EditorId id) const noexcept { return editors_.exists(id); }
    [[nodiscard]] Editor* find(EditorId id) noexcept { return editors_.get(id); }
    [[nodiscard]] const Editor* find(EditorId id) const noexcept { return editors_.get(id); }

    [[nodiscard]] bool clear(EditorId id) {
        if (Editor* editor = find(id)) {
            editor->clear();
            return true;
        }
        return false;
    }
    [[nodiscard]] gauge::GaugeFace* getFace(EditorId id, NodeId faceId) {
        Editor* editor = find(id);
        return editor ? editor->getFace(faceId) : nullptr;
    }
    [[nodiscard]] bool isFace(EditorId id, NodeId faceId) { return getFace(id, faceId) != nullptr; }
    [[nodiscard]] std::size_t faceCount(EditorId id) const {
        const Editor* editor = find(id);
        return editor ? editor->faceCount() : 0;
    }
    [[nodiscard]] NodeId faceAt(EditorId id, std::size_t index) const {
        const Editor* editor = find(id);
        return !editor || index >= editor->faceCount() ? 0 : editor->faceIdAt(index);
    }
    [[nodiscard]] bool undo(EditorId id) { Editor* editor = find(id); return editor && editor->undo(); }
    [[nodiscard]] bool redo(EditorId id) { Editor* editor = find(id); return editor && editor->redo(); }
    [[nodiscard]] bool jumpTo(EditorId id, std::size_t index) { Editor* editor = find(id); return editor && editor->jumpTo(index); }
    [[nodiscard]] std::size_t historyIndex(EditorId id) const { const Editor* editor = find(id); return editor ? editor->historyIndex() : 0; }
    [[nodiscard]] bool canUndo(EditorId id) const { const Editor* editor = find(id); return editor && editor->canUndo(); }
    [[nodiscard]] bool canRedo(EditorId id) const { const Editor* editor = find(id); return editor && editor->canRedo(); }

    [[nodiscard]] Result setPackageInfo(EditorId id, const std::string& name, const std::string& author, const std::string& description);
    [[nodiscard]] Result getPackageInfo(EditorId id);
    [[nodiscard]] Result getAssets(EditorId id);
    [[nodiscard]] Result setAsset(EditorId id, const std::string& name, const std::string& mediaType, const std::string& data);
    [[nodiscard]] Result removeAsset(EditorId id, const std::string& name);
    [[nodiscard]] Result setFaceName(EditorId id, NodeId faceId, const std::string& name);
    [[nodiscard]] Result getFaceName(EditorId id, NodeId faceId);
    [[nodiscard]] Result loadPackage(EditorId id, const std::string& json);
    [[nodiscard]] Result exportPackage(EditorId id);

    [[nodiscard]] Result getHierarchy(EditorId id);
    [[nodiscard]] Result listElementTypes(EditorId id);
    [[nodiscard]] Result listValueIDs(EditorId id);
    [[nodiscard]] Result createFace(EditorId id, const std::string& json, FacePlacement where = FacePlacement{});
    [[nodiscard]] Result removeFace(EditorId id, NodeId faceId);
    [[nodiscard]] Result reorderFace(EditorId id, NodeId faceId, std::size_t index);
    [[nodiscard]] Result createElement(EditorId id, const ElementPlacement& where, const std::string& json);
    [[nodiscard]] Result removeElement(EditorId id, ElementRef element);
    [[nodiscard]] Result reorderElement(EditorId id, ElementRef element, std::size_t index);
    [[nodiscard]] Result moveElement(EditorId id, ElementRef element, const ElementPlacement& where);
    [[nodiscard]] Result replaceElement(EditorId id, ElementRef element, const std::string& json);

    [[nodiscard]] Result setFaceProperty(EditorId id, NodeId faceId, const std::string& path, const std::string& json);
    [[nodiscard]] Result getFaceProperty(EditorId id, NodeId faceId, const std::string& path);
    [[nodiscard]] Result getFacePropertiesMeta(EditorId id, NodeId faceId, const std::string& path = "");
    [[nodiscard]] Result setElementProperty(EditorId id, ElementRef element, const std::string& path, const std::string& json);
    [[nodiscard]] Result getElementProperty(EditorId id, ElementRef element, const std::string& path);
    [[nodiscard]] Result getElementPropertiesMeta(EditorId id, ElementRef element, const std::string& path = "");

    [[nodiscard]] ClipboardState::Kind clipboardKind() const;
    void clearClipboard();
    [[nodiscard]] Result copyFace(EditorId id, NodeId faceId);
    [[nodiscard]] Result cutFace(EditorId id, NodeId faceId);
    [[nodiscard]] Result pasteFace(EditorId id, FacePlacement where = FacePlacement{});
    [[nodiscard]] Result copyElement(EditorId id, ElementRef element);
    [[nodiscard]] Result cutElement(EditorId id, ElementRef element);
    [[nodiscard]] Result pasteElement(EditorId id, const ElementPlacement& where);
    [[nodiscard]] Result pasteToReplaceElement(EditorId id, ElementRef element);

    [[nodiscard]] Result getHistory(EditorId id);

    [[nodiscard]] ClipboardState& clipboard() noexcept { return clipboard_; }
    [[nodiscard]] const ClipboardState& clipboard() const noexcept { return clipboard_; }

private:
    HandlePool<Editor, EditorId> editors_;
    ClipboardState clipboard_;
};

} // namespace mg::editor
