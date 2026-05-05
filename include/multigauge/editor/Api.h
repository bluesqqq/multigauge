#pragma once

#include <cstddef>
#include <memory>
#include <string>

#include <multigauge/editor/Types.h>
#include <multigauge/editor/Result.h>

namespace mg::editor {

struct ClipboardSummary;

//----------[ EDITORS ]----------//

/// Creates a new editor instance.
EditorId create();

/// Destroys the editor instance with `id`.
bool destroy(EditorId);

/// Returns whether an editor instance with `id` exists.
bool exists(EditorId);

//----------[ DOCUMENT ]----------//

/// Clears the loaded document, history, and node IDs for the editor.
bool clear(EditorId);

/// Loads a document from a JSON array of faces.
Result loadDocument(EditorId, const std::string& json);

/// Saves the current document as a JSON array.
Result saveDocument(EditorId);

//----------[ QUERIES ]----------//

/// Returns the face and element hierarchy as a flat JSON payload.
/// @return `{ "roots": [uint...], "nodes": { "id": { "kind": string, "name": string, "children": [uint...] }, ... } }`
Result getHierarchy(EditorId);

/// Returns the available element type descriptors.
/// @return `[ { "name": string, "type": string }, ... ]`
Result listElementTypes(EditorId);

/// Returns the available value IDs.
/// @return `[ string, ... ]`
Result listValueIDs(EditorId);

/// Returns whether a face or element with `id` exists.
bool hasNode(EditorId, NodeId id);

/// Returns whether `id` refers to a face.
bool isFace(EditorId, NodeId id);

/// Returns whether `id` refers to an element.
bool isElement(EditorId, NodeId id);

//----------[ GAUGE FACES ]----------//

/// Creates a new face in the face list.
/// @note `where.index` appends when set to `Append` or greater than the face count.
/// @return `{ "id": uint }` for the inserted face.
Result createFace(EditorId, const std::string& json, FacePlacement where = FacePlacement());

/// Removes a face from the face list.
Result removeFace(EditorId, NodeId faceId);

/// Reorders a face within the face list.
/// @note `index` is zero-based and clamps to the valid face range.
Result reorderFace(EditorId, NodeId faceId, std::size_t index);

/// Returns the number of faces in the editor.
std::size_t faceCount(EditorId);

/// Returns the face ID at `index`.
NodeId faceAt(EditorId, std::size_t index);

//----------[ ELEMENTS ]----------//

/// Creates an element under a face or element parent from JSON.
/// @note `where.parentId` may refer to either a face or an element.
/// @note `where.index` appends when set to `Append` or greater than the parent child count.
/// @return `{ "id": uint, "parentId": uint }` for the inserted element.
Result createElement(EditorId, const ElementPlacement& where, const std::string& json);

/// Removes an element from its current parent.
Result removeElement(EditorId, NodeId elementId);

/// Reorders an element within its current parent.
/// @note `index` is zero-based and clamps to the valid sibling range.
Result reorderElement(EditorId, NodeId elementId, std::size_t index);

/// Moves an element to a new face or element parent.
/// @note `where.parentId` may refer to either a face or an element.
/// @note `where.index` appends when set to `Append` or greater than the destination child count.
/// @return `{ "id": uint, "parentId": uint }` for the moved element.
Result moveElement(EditorId, NodeId elementId, const ElementPlacement& where);

/// Replaces an element with a new element loaded from JSON.
/// @return `{ "id": uint }` for the replaced element.
Result replaceElement(EditorId, NodeId elementId, const std::string& json);

//----------[ PROPERTIES ]----------//

/// Sets a property on a face or element from JSON.
/// @param path Dotted property path such as `"style.margin.left"`.
/// @return `{ "id": uint, "path": string, "value": any }`.
Result setProperty(EditorId, NodeId id, const std::string& path, const std::string& json);

/// Gets a property from a face or element.
/// @param path Dotted property path such as `"style.margin.left"`.
/// @return `{ "id": uint, "path": string, "value": any }`.
Result getProperty(EditorId, NodeId id, const std::string& path);

/// Gets property metadata from a face or element.
/// @param path Dotted property path, or empty to describe the whole object.
/// @return `{ "id": uint, "meta": object }` for an empty `path`, or
/// `{ "id": uint, "path": string, "meta": object }` for a resolved property.
Result getPropertiesMeta(EditorId, NodeId id, const std::string& path = "");

//----------[ CLIPBOARD ]----------//

/// Returns the current clipboard summary.
/// @return A summary of the clipboard contents.
ClipboardSummary getClipboardSummary(EditorId);

/// Clears the clipboard for the editor.
void clearClipboard(EditorId);

/// Copies a face into the clipboard.
Result copyFace(EditorId, NodeId faceId);

/// Cuts a face into the clipboard.
Result cutFace(EditorId, NodeId faceId);

/// Pastes a face from the clipboard.
Result pasteFace(EditorId, FacePlacement where = FacePlacement());

/// Copies an element into the clipboard.
Result copyElement(EditorId, NodeId elementId);

/// Cuts an element into the clipboard.
Result cutElement(EditorId, NodeId elementId);

/// Pastes an element from the clipboard.
Result pasteElement(EditorId, const ElementPlacement& where);

/// Replaces an element with the element stored in the clipboard.
Result pasteToReplaceElement(EditorId, NodeId elementId);

//----------[ HISTORY ]----------//

/// Undoes the most recent committed edit.
bool undo(EditorId);

/// Redoes the next committed edit.
bool redo(EditorId);

/// Jumps the history cursor to `index`.
bool jumpTo(EditorId, std::size_t index);

std::size_t historyIndex(EditorId);

/// Returns the history command names in order.
/// @return `[ string, ... ]`
Result getHistory(EditorId);

}
