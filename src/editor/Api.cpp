#include <multigauge/editor/Api.h>

#include <cstdint>

#include <multigauge/container/HandlePool.h>
#include <multigauge/editor/Clipboard.h>
#include <multigauge/editor/Editor.h>
#include <multigauge/value/ValueRegistry.h>

namespace mg::editor {
namespace {
HandlePool<Editor, EditorId> editors;
ClipboardState clipboard;

Editor* editor(EditorId id) {
    return editors.get(id);
}

Result invalidEditor() {
    return Error("Invalid editor ID");
}

Result jsonResult(const std::string& json) {
    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject(
               [&](json::ObjectWriter& object) { return object.write("json", json); })
               ? std::move(result)
               : Error("Failed to build JSON result");
}

Result packageInfoResult(const Editor::PackageInfo& info) {
    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        return object.write("name", info.name) && object.write("author", info.author) &&
               object.write("description", info.description);
    })
               ? std::move(result)
               : Error("Failed to build package result");
}
} // namespace

EditorId create() {
    return editors.emplace();
}

bool destroy(EditorId id) {
    return editors.remove(id);
}

bool exists(EditorId id) {
    return editor(id) != nullptr;
}

bool clear(EditorId id) {
    if (auto* value = editor(id)) {
        value->clear();
        return true;
    }
    return false;
}

Result setPackageInfo(
    EditorId id,
    const std::string& name,
    const std::string& author,
    const std::string& description
) {
    auto* value = editor(id);
    if (!value) return invalidEditor();
    const Editor::PackageInfo info{name, author, description};
    return value->setPackageInfo(info)
               ? packageInfoResult(value->packageInfo())
               : Error("Failed to set package info");
}

Result getPackageInfo(EditorId id) {
    auto* value = editor(id);
    return value ? packageInfoResult(value->packageInfo()) : invalidEditor();
}

Result setFaceName(
    EditorId id,
    NodeId faceId,
    const std::string& name
) {
    auto* value = editor(id);
    if (!value) return invalidEditor();
    return value->setFaceName(faceId, name) ? OkObject() : Error("Invalid face ID");
}

Result getFaceName(
    EditorId id,
    NodeId faceId
) {
    auto* value = editor(id);
    if (!value) return invalidEditor();
    if (!value->getFace(faceId)) return Error("Invalid face ID");
    Result result = OkObject();
    json::Writer writer = result.data.writer();
    return writer.writeObject([&](json::ObjectWriter& object) {
        return object.write("id", static_cast<std::uint64_t>(faceId)) &&
               object.write("name", value->getFaceName(faceId));
    })
               ? std::move(result)
               : Error("Failed to build face result");
}

Result loadPackage(
    EditorId id,
    const std::string& text
) {
    auto* value = editor(id);
    return !value ? invalidEditor()
                  : (value->loadPackage(text) ? OkObject() : Error("Invalid package JSON"));
}

Result exportPackage(EditorId id) {
    auto* value = editor(id);
    return value ? jsonResult(value->exportPackage()) : invalidEditor();
}

gauge::GaugeFace* getFace(
    EditorId id,
    NodeId faceId
) {
    auto* value = editor(id);
    return value ? value->getFace(faceId) : nullptr;
}

bool isFace(
    EditorId id,
    NodeId faceId
) {
    return getFace(id, faceId) != nullptr;
}

std::size_t faceCount(EditorId id) {
    auto* value = editor(id);
    return value ? value->faceCount() : 0;
}

NodeId faceAt(
    EditorId id,
    std::size_t index
) {
    auto* value = editor(id);
    return !value || index >= value->faceCount() ? 0 : value->faceIdAt(index);
}

Result getHierarchy(EditorId id) {
    auto* value = editor(id);
    return value ? value->getHierarchy() : invalidEditor();
}

Result listElementTypes(EditorId id) {
    auto* value = editor(id);
    return value ? value->listElementTypes() : invalidEditor();
}

Result listValueIDs(EditorId id) {
    if (!editor(id)) return invalidEditor();

    Result result = OkArray();
    json::Writer writer = result.data.writer();
    return writer.writeArray([&](json::ArrayWriter& entries) {
        bool written = true;
        ValueRegistry::forEach([&](ValueHandle handle) {
            written = written && entries.write(ValueRegistry::id(handle));
        });
        return written;
    })
               ? std::move(result)
               : Error("Failed to list value IDs");
}

Result createFace(
    EditorId id,
    const std::string& text,
    FacePlacement where
) {
    auto* value = editor(id);
    return value ? value->createFace(text, where) : invalidEditor();
}

Result removeFace(
    EditorId id,
    NodeId faceId
) {
    auto* value = editor(id);
    return value ? value->removeFace(faceId) : invalidEditor();
}

Result reorderFace(
    EditorId id,
    NodeId faceId,
    std::size_t index
) {
    auto* value = editor(id);
    return value ? value->reorderFace(faceId, index) : invalidEditor();
}

Result createElement(
    EditorId id,
    const ElementPlacement& where,
    const std::string& text
) {
    auto* value = editor(id);
    return value ? value->createElement(where, text) : invalidEditor();
}

Result removeElement(
    EditorId id,
    ElementRef ref
) {
    auto* value = editor(id);
    return value ? value->removeElement(ref) : invalidEditor();
}

Result reorderElement(
    EditorId id,
    ElementRef ref,
    std::size_t index
) {
    auto* value = editor(id);
    return value ? value->reorderElement(ref, index) : invalidEditor();
}

Result moveElement(
    EditorId id,
    ElementRef ref,
    const ElementPlacement& where
) {
    auto* value = editor(id);
    return value ? value->moveElement(ref, where) : invalidEditor();
}

Result replaceElement(
    EditorId id,
    ElementRef ref,
    const std::string& text
) {
    auto* value = editor(id);
    return value ? value->replaceElement(ref, text) : invalidEditor();
}

Result setFaceProperty(
    EditorId id,
    NodeId faceId,
    const std::string& path,
    const std::string& text
) {
    auto* value = editor(id);
    return value ? value->setFaceProperty(faceId, path, text) : invalidEditor();
}

Result getFaceProperty(
    EditorId id,
    NodeId faceId,
    const std::string& path
) {
    auto* value = editor(id);
    return value ? value->getFaceProperty(faceId, path) : invalidEditor();
}

Result getFacePropertiesMeta(
    EditorId id,
    NodeId faceId,
    const std::string& path
) {
    auto* value = editor(id);
    return value ? value->getFacePropertiesMeta(faceId, path) : invalidEditor();
}

Result setElementProperty(
    EditorId id,
    ElementRef ref,
    const std::string& path,
    const std::string& text
) {
    auto* value = editor(id);
    return value ? value->setElementProperty(ref, path, text) : invalidEditor();
}

Result getElementProperty(
    EditorId id,
    ElementRef ref,
    const std::string& path
) {
    auto* value = editor(id);
    return value ? value->getElementProperty(ref, path) : invalidEditor();
}

Result getElementPropertiesMeta(
    EditorId id,
    ElementRef ref,
    const std::string& path
) {
    auto* value = editor(id);
    return value ? value->getElementPropertiesMeta(ref, path) : invalidEditor();
}

ClipboardSummary getClipboardSummary(EditorId) {
    return {clipboard.kind};
}

void clearClipboard(EditorId) {
    clipboard.clear();
}

Result copyFace(
    EditorId id,
    NodeId faceId
) {
    auto* value = editor(id);
    if (!value) return invalidEditor();
    Result result = value->serializeFace(faceId);
    if (!result.ok) return result;
    std::string_view text;
    if (!result.data.root().member("json").read(text)) return Error("Invalid serialized face");
    clipboard = {ClipboardState::Kind::Face, std::string(text)};
    return OkObject();
}

Result cutFace(
    EditorId id,
    NodeId faceId
) {
    Result result = copyFace(id, faceId);
    if (!result.ok) return std::move(result);
    return removeFace(id, faceId);
}

Result pasteFace(
    EditorId id,
    FacePlacement where
) {
    auto* value = editor(id);
    if (!value) return invalidEditor();
    return clipboard.kind == ClipboardState::Kind::Face
               ? value->createFace(clipboard.json, where)
               : Error("Clipboard does not contain a face");
}

Result copyElement(
    EditorId id,
    ElementRef ref
) {
    auto* value = editor(id);
    if (!value) return invalidEditor();
    Result result = value->serializeElement(ref);
    if (!result.ok) return result;
    std::string_view text;
    if (!result.data.root().member("json").read(text)) return Error("Invalid serialized element");
    clipboard = {ClipboardState::Kind::Element, std::string(text)};
    return OkObject();
}

Result cutElement(
    EditorId id,
    ElementRef ref
) {
    Result result = copyElement(id, ref);
    if (!result.ok) return std::move(result);
    return removeElement(id, ref);
}

Result pasteElement(
    EditorId id,
    const ElementPlacement& where
) {
    auto* value = editor(id);
    if (!value) return invalidEditor();
    return clipboard.kind == ClipboardState::Kind::Element
               ? value->createElement(where, clipboard.json)
               : Error("Clipboard does not contain an element");
}

Result pasteToReplaceElement(
    EditorId id,
    ElementRef ref
) {
    auto* value = editor(id);
    if (!value) return invalidEditor();
    return clipboard.kind == ClipboardState::Kind::Element
               ? value->replaceElement(ref, clipboard.json)
               : Error("Clipboard does not contain an element");
}

bool undo(EditorId id) {
    auto* value = editor(id);
    return value && value->undo();
}

bool redo(EditorId id) {
    auto* value = editor(id);
    return value && value->redo();
}

bool jumpTo(
    EditorId id,
    std::size_t index
) {
    auto* value = editor(id);
    return value && value->jumpTo(index);
}

std::size_t historyIndex(EditorId id) {
    auto* value = editor(id);
    return value ? value->historyIndex() : 0;
}

Result getHistory(EditorId id) {
    auto* value = editor(id);
    return value ? value->getHistory() : invalidEditor();
}

bool canUndo(EditorId id) {
    auto* value = editor(id);
    return value && value->canUndo();
}

bool canRedo(EditorId id) {
    auto* value = editor(id);
    return value && value->canRedo();
}

} // namespace mg::editor
