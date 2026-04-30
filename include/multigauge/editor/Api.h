#pragma once

#include <memory>
#include <string>

#include <multigauge/editor/Types.h>
#include <multigauge/editor/Result.h>

namespace mg::editor {

struct ClipboardSummary;

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

bool hasNode(EditorId, NodeId id);
bool isFace(EditorId, NodeId id);
bool isElement(EditorId, NodeId id);

//----------[ GAUGE FACES ]----------//

Result createFace(EditorId, const std::string& json, FacePlacement where = FacePlacement());
Result removeFace(EditorId, NodeId faceId);
Result reorderFace(EditorId, NodeId faceId, std::size_t index);

std::size_t faceCount(EditorId);
NodeId faceAt(EditorId, std::size_t index);

//----------[ ELEMENTS ]----------//

Result createElement(EditorId, const ElementPlacement& where, const std::string& json);
Result removeElement(EditorId, NodeId elementId);
Result reorderElement(EditorId, NodeId elementId, std::size_t index);
Result moveElement(EditorId, NodeId elementId, const ElementPlacement& where);
Result replaceElement(EditorId, NodeId elementId, const std::string& json);

//----------[ PROPERTIES ]----------//

Result setProperty(EditorId, NodeId id, const std::string& path, const std::string& json);
Result getProperty(EditorId, NodeId id, const std::string& path);
Result getPropertiesMeta(EditorId, NodeId id, const std::string& path = "");

//----------[ CLIPBOARD ]----------//

ClipboardSummary getClipboardSummary(EditorId);
void clearClipboard(EditorId);

Result copyFace(EditorId, NodeId faceId);
Result cutFace(EditorId, NodeId faceId);
Result pasteFace(EditorId, FacePlacement where = FacePlacement());
Result copyElement(EditorId, NodeId elementId);
Result cutElement(EditorId, NodeId elementId);
Result pasteElement(EditorId, const ElementPlacement& where);
Result pasteToReplaceElement(EditorId, NodeId elementId);

//----------[ HISTORY ]----------//

bool undo(EditorId);
bool redo(EditorId);
Result getHistory(EditorId);

}
