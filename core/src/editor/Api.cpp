#include <multigauge/editor/Api.h>
#include <multigauge/editor/Clipboard.h>
#include <multigauge/editor/Editor.h>

#include <multigauge/HandlePool.h>

namespace mg::editor {

using ::mg::Result;

namespace {

HandlePool<Editor> editors;
ClipboardState clipboard;

Editor* getEditor(EditorId EditorId) {
    return editors.get(EditorId);
}

Result invalidEditorId() {
    return Error("Invalid editor EditorId");
}

Result saveJsonResult(const std::string& json) {
    Result result = OkObject();
    auto& allocator = result.data.GetAllocator();
    result.data.AddMember("json", rapidjson::Value(json.c_str(), allocator), allocator);
    return result;
}

Result packageInfoResult(const Editor::PackageInfo& info) {
    Result result = OkObject();
    auto& allocator = result.data.GetAllocator();
    result.data.AddMember("name", rapidjson::Value(info.name.c_str(), allocator), allocator);
    result.data.AddMember("author", rapidjson::Value(info.author.c_str(), allocator), allocator);
    result.data.AddMember("description", rapidjson::Value(info.description.c_str(), allocator), allocator);
    return result;
}

Result faceNameResult(NodeId faceId, const std::string& name) {
    Result result = OkObject();
    auto& allocator = result.data.GetAllocator();
    result.data.AddMember("id", faceId, allocator);
    result.data.AddMember("name", rapidjson::Value(name.c_str(), allocator), allocator);
    return result;
}

}

gauge::GaugeFace* getFace(EditorId EditorId, NodeId faceId) {
    Editor* editor = getEditor(EditorId);
    if (!editor || !editor->isFace(faceId)) return nullptr;

    for (std::size_t i = 0; i < editor->faceCount(); ++i) {
        if (editor->faceAt(i) && editor->idOf(editor->faceAt(i)) == faceId) {
            return editor->faceAt(i);
        }
    }

    return nullptr;
}

EditorId create() {
    return editors.emplace();
}

bool destroy(EditorId EditorId) {
    return editors.remove(EditorId);
}

bool exists(EditorId EditorId) {
    return getEditor(EditorId) != nullptr;
}

bool clear(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return false;
    editor->clear();
    return true;
}

Result setPackageInfo(EditorId EditorId, const std::string& name, const std::string& author, const std::string& description) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return invalidEditorId();
    if (!editor->setPackageInfo(name, author, description)) return Error("Failed to set package info");
    return packageInfoResult(editor->getPackageInfo());
}

Result getPackageInfo(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return invalidEditorId();
    return packageInfoResult(editor->getPackageInfo());
}

Result setFaceName(EditorId EditorId, NodeId faceId, const std::string& name) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return invalidEditorId();
    if (!editor->setFaceName(faceId, name)) return Error("Invalid face id");
    return faceNameResult(faceId, editor->getFaceName(faceId));
}

Result getFaceName(EditorId EditorId, NodeId faceId) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return invalidEditorId();
    if (!editor->isFace(faceId)) return Error("Invalid face id");
    return faceNameResult(faceId, editor->getFaceName(faceId));
}

Result loadPackage(EditorId EditorId, const std::string& json) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return invalidEditorId();
    if (!editor->loadPackage(json)) return Error("Invalid JSON");
    return OkObject();
}

Result exportPackage(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return invalidEditorId();
    return saveJsonResult(editor->exportPackage());
}

Result getHierarchy(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->getHierarchy() : invalidEditorId();
}

Result getHistory(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->getHistory() : invalidEditorId();
}

ClipboardSummary getClipboardSummary(EditorId EditorId) {
    (void)EditorId;
    return { clipboard.kind };
}

void clearClipboard(EditorId EditorId) {
    (void)EditorId;
    clipboard.clear();
}

std::size_t faceCount(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->faceCount() : 0;
}

NodeId faceAt(EditorId EditorId, std::size_t index) {
    Editor* editor = getEditor(EditorId);
    if (!editor || index >= editor->faceCount()) return 0;
    return editor->idOf(editor->faceAt(index));
}

Result listElementTypes(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->listElementTypes() : invalidEditorId();
}

Result listValueIDs(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->listValueIDs() : invalidEditorId();
}

bool hasNode(EditorId EditorId, NodeId id) {
    Editor* editor = getEditor(EditorId);
    return editor && editor->hasNode(id);
}

bool isFace(EditorId EditorId, NodeId id) {
    Editor* editor = getEditor(EditorId);
    return editor && editor->isFace(id);
}

bool isElement(EditorId EditorId, NodeId id) {
    Editor* editor = getEditor(EditorId);
    return editor && editor->isElement(id);
}

Result createFace(EditorId EditorId, const std::string& json, FacePlacement where) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->createFace(json, where) : invalidEditorId();
}

Result removeFace(EditorId EditorId, NodeId faceId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->removeFace(faceId) : invalidEditorId();
}

Result reorderFace(EditorId EditorId, NodeId faceId, std::size_t index) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->reorderFace(faceId, index) : invalidEditorId();
}

Result createElement(EditorId EditorId, const ElementPlacement& where, const std::string& json) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->createElement(where, json) : invalidEditorId();
}

Result removeElement(EditorId EditorId, NodeId elementId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->removeElement(elementId) : invalidEditorId();
}

Result reorderElement(EditorId EditorId, NodeId elementId, std::size_t index) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->reorderElement(elementId, index) : invalidEditorId();
}

Result moveElement(EditorId EditorId, NodeId elementId, const ElementPlacement& where) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->moveElement(elementId, where) : invalidEditorId();
}

Result replaceElement(EditorId EditorId, NodeId elementId, const std::string& json) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->replaceElement(elementId, json) : invalidEditorId();
}

Result setProperty(EditorId EditorId, NodeId id, const std::string& path, const std::string& json) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->setProperty(id, path, json) : invalidEditorId();
}

Result getProperty(EditorId EditorId, NodeId id, const std::string& path) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->getProperty(id, path) : invalidEditorId();
}

Result getPropertiesMeta(EditorId EditorId, NodeId id, const std::string& path) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->getPropertiesMeta(id, path) : invalidEditorId();
}

Result copyFace(EditorId EditorId, NodeId faceId) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return invalidEditorId();

    Result serialized = editor->serializeFace(faceId);
    if (!serialized.ok) return serialized;

    clipboard.kind = ClipboardState::Kind::Face;
    clipboard.json = serialized.data["json"].GetString();
    return OkObject();
}

Result cutFace(EditorId EditorId, NodeId faceId) {
    Result copied = copyFace(EditorId, faceId);
    if (!copied.ok) return copied;

    Editor* editor = getEditor(EditorId);
    return editor ? editor->removeFace(faceId) : invalidEditorId();
}

Result pasteFace(EditorId EditorId, FacePlacement where) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return invalidEditorId();

    if (clipboard.kind != ClipboardState::Kind::Face) {
        return Error("Clipboard does not contain a face");
    }

    return editor->createFace(clipboard.json, where);
}

Result copyElement(EditorId EditorId, NodeId elementId) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return invalidEditorId();

    Result serialized = editor->serializeElement(elementId);
    if (!serialized.ok) return serialized;

    clipboard.kind = ClipboardState::Kind::Element;
    clipboard.json = serialized.data["json"].GetString();
    return OkObject();
}

Result cutElement(EditorId EditorId, NodeId elementId) {
    Result copied = copyElement(EditorId, elementId);
    if (!copied.ok) return copied;

    Editor* editor = getEditor(EditorId);
    return editor ? editor->removeElement(elementId) : invalidEditorId();
}

Result pasteElement(EditorId EditorId, const ElementPlacement& where) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return invalidEditorId();

    if (clipboard.kind != ClipboardState::Kind::Element) {
        return Error("Clipboard does not contain an element");
    }

    return editor->createElement(where, clipboard.json);
}

Result pasteToReplaceElement(EditorId EditorId, NodeId elementId) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return invalidEditorId();

    if (clipboard.kind != ClipboardState::Kind::Element) {
        return Error("Clipboard does not contain an element");
    }

    return editor->replaceElement(elementId, clipboard.json);
}

bool undo(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    return editor && editor->undo();
}

bool redo(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    return editor && editor->redo();
}

bool jumpTo(EditorId EditorId, std::size_t index) {
    Editor* editor = getEditor(EditorId);
    return editor && editor->jumpTo(index);
}

std::size_t historyIndex(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->historyIndex() : 0;
}

}
