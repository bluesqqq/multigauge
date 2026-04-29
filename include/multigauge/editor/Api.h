#pragma once

#include <multigauge/editor/Result.h>
#include <multigauge/editor/Editor.h>

namespace mg::editor {
using EditorId = uint32_t;

EditorId create();
bool destroy(EditorId);
bool exists(EditorId);

bool clear(EditorId);
Result loadDocument(EditorId, const std::string& json);
Result saveDocument(EditorId);

Result getHierarchy(EditorId);
Result getHistory(EditorId);
ClipboardSummary getClipboardSummary(EditorId);
void clearClipboard(EditorId);
std::size_t faceCount(EditorId);
Editor::Id faceAt(EditorId, std::size_t index);
Result listElementTypes(EditorId);
Result listValueIDs(EditorId);
bool hasNode(EditorId, Editor::Id id);
bool isFace(EditorId, Editor::Id id);
bool isElement(EditorId, Editor::Id id);
gauge::GaugeFace* getFace(EditorId, Editor::Id faceId);
const gauge::GaugeFace* getFace(EditorId, Editor::Id faceId, std::nullptr_t);

Result createFace(EditorId, const std::string& json, Editor::FacePlacement where = Editor::FacePlacement());
Result removeFace(EditorId, Editor::Id faceId);
Result reorderFace(EditorId, Editor::Id faceId, std::size_t index);

Result createElement(EditorId, const Editor::ElementPlacement& where, const std::string& json);
Result removeElement(EditorId, Editor::Id elementId);
Result reorderElement(EditorId, Editor::Id elementId, std::size_t index);
Result moveElement(EditorId, Editor::Id elementId, const Editor::ElementPlacement& where);
Result replaceElementFromJson(EditorId, Editor::Id elementId, const std::string& json);

Result setProperty(EditorId, Editor::Id id, const std::string& path, const std::string& json);
Result getProperty(EditorId, Editor::Id id, const std::string& path);
Result getPropertiesMeta(EditorId, Editor::Id id, const std::string& path = "");

Result copyFace(EditorId, Editor::Id faceId);
Result cutFace(EditorId, Editor::Id faceId);
Result pasteFace(EditorId, Editor::FacePlacement where = Editor::FacePlacement());
Result copyElement(EditorId, Editor::Id elementId);
Result cutElement(EditorId, Editor::Id elementId);
Result pasteElement(EditorId, const Editor::ElementPlacement& where);
Result pasteToReplaceElement(EditorId, Editor::Id elementId);

bool undo(EditorId);
bool redo(EditorId);
}
