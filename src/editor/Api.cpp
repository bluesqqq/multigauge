#include <multigauge/editor/Api.h>

#include <multigauge/HandlePool.h>

namespace mg::editor {

namespace {

HandlePool<Editor> editors;

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

Result loadDocument(EditorId EditorId, const std::string& json) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return invalidEditorId();
    editor->loadDocument(json);
    return OkObject();
}

Result saveDocument(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    if (!editor) return invalidEditorId();
    return saveJsonResult(editor->saveDocument());
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
    Editor* editor = getEditor(EditorId);
    return editor ? editor->getClipboardSummary() : ClipboardSummary{};
}

void clearClipboard(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    if (editor) editor->clearClipboard();
}

std::size_t faceCount(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->faceCount() : 0;
}

Editor::Id faceAt(EditorId EditorId, std::size_t index) {
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

bool hasNode(EditorId EditorId, Editor::Id id) {
    Editor* editor = getEditor(EditorId);
    return editor && editor->hasNode(id);
}

bool isFace(EditorId EditorId, Editor::Id id) {
    Editor* editor = getEditor(EditorId);
    return editor && editor->isFace(id);
}

bool isElement(EditorId EditorId, Editor::Id id) {
    Editor* editor = getEditor(EditorId);
    return editor && editor->isElement(id);
}

gauge::GaugeFace* getFace(EditorId EditorId, Editor::Id faceId) {
    Editor* editor = getEditor(EditorId);
    if (!editor || !editor->isFace(faceId)) return nullptr;

    for (std::size_t i = 0; i < editor->faceCount(); ++i) {
        gauge::GaugeFace* face = editor->faceAt(i);
        if (face && editor->idOf(face) == faceId) return face;
    }

    return nullptr;
}

const gauge::GaugeFace* getFace(EditorId EditorId, Editor::Id faceId, std::nullptr_t) {
    return getFace(EditorId, faceId);
}

Result createFace(EditorId EditorId, const std::string& json, Editor::FacePlacement where) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->createFace(json, where) : invalidEditorId();
}

Result removeFace(EditorId EditorId, Editor::Id faceId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->removeFace(faceId) : invalidEditorId();
}

Result reorderFace(EditorId EditorId, Editor::Id faceId, std::size_t index) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->reorderFace(faceId, index) : invalidEditorId();
}

Result createElement(EditorId EditorId, const Editor::ElementPlacement& where, const std::string& json) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->createElement(where, json) : invalidEditorId();
}

Result removeElement(EditorId EditorId, Editor::Id elementId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->removeElement(elementId) : invalidEditorId();
}

Result reorderElement(EditorId EditorId, Editor::Id elementId, std::size_t index) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->reorderElement(elementId, index) : invalidEditorId();
}

Result moveElement(EditorId EditorId, Editor::Id elementId, const Editor::ElementPlacement& where) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->moveElement(elementId, where) : invalidEditorId();
}

Result replaceElementFromJson(EditorId EditorId, Editor::Id elementId, const std::string& json) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->replaceElementFromJson(elementId, json) : invalidEditorId();
}

Result setProperty(EditorId EditorId, Editor::Id id, const std::string& path, const std::string& json) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->setProperty(id, path, json) : invalidEditorId();
}

Result getProperty(EditorId EditorId, Editor::Id id, const std::string& path) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->getProperty(id, path) : invalidEditorId();
}

Result getPropertiesMeta(EditorId EditorId, Editor::Id id, const std::string& path) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->getPropertiesMeta(id, path) : invalidEditorId();
}

Result copyFace(EditorId EditorId, Editor::Id faceId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->copyFace(faceId) : invalidEditorId();
}

Result cutFace(EditorId EditorId, Editor::Id faceId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->cutFace(faceId) : invalidEditorId();
}

Result pasteFace(EditorId EditorId, Editor::FacePlacement where) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->pasteFace(where) : invalidEditorId();
}

Result copyElement(EditorId EditorId, Editor::Id elementId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->copyElement(elementId) : invalidEditorId();
}

Result cutElement(EditorId EditorId, Editor::Id elementId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->cutElement(elementId) : invalidEditorId();
}

Result pasteElement(EditorId EditorId, const Editor::ElementPlacement& where) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->pasteElement(where) : invalidEditorId();
}

Result pasteToReplaceElement(EditorId EditorId, Editor::Id elementId) {
    Editor* editor = getEditor(EditorId);
    return editor ? editor->pasteToReplaceElement(elementId) : invalidEditorId();
}

bool undo(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    return editor && editor->undo();
}

bool redo(EditorId EditorId) {
    Editor* editor = getEditor(EditorId);
    return editor && editor->redo();
}

}
