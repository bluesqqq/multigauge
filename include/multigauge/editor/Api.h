#pragma once

#include <multigauge/editor/Result.h>
#include <multigauge/editor/Editor.h>

namespace mg::editor {

using EditorId = uint32_t;

//----------[ EDITORS ]----------//

EditorId create();
bool destroy(EditorId);
bool exists(EditorId);

//----------[ DOCUMENT ]----------//

bool clear(EditorId);
Result loadDocument(EditorId, const std::string& json);
Result saveDocument(EditorId);

//----------[ QUERIES ]----------//

Result getHierarchy(EditorId);
Result listElementTypes(EditorId);
Result listValueIDs(EditorId);

bool hasNode(EditorId, Editor::Id id);
bool isFace(EditorId, Editor::Id id);
bool isElement(EditorId, Editor::Id id);

//----------[ GAUGE FACES ]----------//

Result createFace(EditorId, const std::string& json, Editor::FacePlacement where = Editor::FacePlacement());
Result removeFace(EditorId, Editor::Id faceId);
Result reorderFace(EditorId, Editor::Id faceId, std::size_t index);

std::size_t faceCount(EditorId);
Editor::Id faceAt(EditorId, std::size_t index);

gauge::GaugeFace* getFace(EditorId, Editor::Id faceId);
const gauge::GaugeFace* getFace(EditorId, Editor::Id faceId, std::nullptr_t);

//----------[ ELEMENTS ]----------//

Result createElement(EditorId, const Editor::ElementPlacement& where, const std::string& json);
Result removeElement(EditorId, Editor::Id elementId);
Result reorderElement(EditorId, Editor::Id elementId, std::size_t index);
Result moveElement(EditorId, Editor::Id elementId, const Editor::ElementPlacement& where);
Result replaceElementFromJson(EditorId, Editor::Id elementId, const std::string& json);

//----------[ PROPERTIES ]----------//

Result setProperty(EditorId, Editor::Id id, const std::string& path, const std::string& json);
Result getProperty(EditorId, Editor::Id id, const std::string& path);
Result getPropertiesMeta(EditorId, Editor::Id id, const std::string& path = "");

//----------[ CLIPBOARD ]----------//

ClipboardSummary getClipboardSummary(EditorId);
void clearClipboard(EditorId);

Result copyFace(EditorId, Editor::Id faceId);
Result cutFace(EditorId, Editor::Id faceId);
Result pasteFace(EditorId, Editor::FacePlacement where = Editor::FacePlacement());
Result copyElement(EditorId, Editor::Id elementId);
Result cutElement(EditorId, Editor::Id elementId);
Result pasteElement(EditorId, const Editor::ElementPlacement& where);
Result pasteToReplaceElement(EditorId, Editor::Id elementId);

//----------[ HISTORY ]----------//

bool undo(EditorId);
bool redo(EditorId);
Result getHistory(EditorId);

}
