#pragma once

#include <cstddef>
#include <string>

#include <multigauge/Result.h>
#include <multigauge/editor/Types.h>

namespace mg::gauge {
class GaugeFace;
}

namespace mg::editor {

struct ClipboardSummary;

/// @brief Clears an editor's package and history state.
bool clear(EditorId id);

//----------[ PACKAGE ]----------//

/// @brief Updates package metadata.
Result setPackageInfo(
    EditorId id,
    const std::string& name,
    const std::string& author,
    const std::string& description
);

/// @brief Returns package metadata.
Result getPackageInfo(EditorId id);

/// @brief Returns the editor package's embedded asset array.
Result getAssets(EditorId id);

/// @brief Adds or replaces one editor package asset from its individual fields.
Result setAsset(EditorId id, const std::string& name, const std::string& mediaType, const std::string& data);

/// @brief Removes an unreferenced editor package asset by its logical name.
Result removeAsset(EditorId id, const std::string& name);

/// @brief Updates one face name.
Result setFaceName(
    EditorId id,
    NodeId faceId,
    const std::string& name
);

/// @brief Returns one face name.
Result getFaceName(
    EditorId id,
    NodeId faceId
);

/// @brief Loads a package document.
Result loadPackage(
    EditorId id,
    const std::string& json
);

/// @brief Exports a package document.
Result exportPackage(EditorId id);

//----------[ FACES ]----------//

/// @brief Returns a borrowed face pointer for native rendering.
/// @details The pointer remains valid until its face is removed, the package is cleared or
/// loaded, a history operation restores a snapshot, or the editor is destroyed.
gauge::GaugeFace* getFace(
    EditorId id,
    NodeId faceId
);

/// @brief Tests whether a face ID belongs to an editor.
bool isFace(
    EditorId id,
    NodeId faceId
);

/// @brief Returns the number of faces in an editor.
std::size_t faceCount(EditorId id);

/// @brief Returns the face ID at an index.
NodeId faceAt(
    EditorId id,
    std::size_t index
);

/// @brief Creates a face from serialized JSON.
Result createFace(
    EditorId id,
    const std::string& json,
    FacePlacement where = FacePlacement{}
);

/// @brief Removes a face.
Result removeFace(
    EditorId id,
    NodeId faceId
);

/// @brief Reorders a face.
Result reorderFace(
    EditorId id,
    NodeId faceId,
    std::size_t index
);

//----------[ ELEMENTS ]----------//

/// @brief Returns the editor hierarchy as JSON.
Result getHierarchy(EditorId id);

/// @brief Lists registered element types.
Result listElementTypes(EditorId id);

/// @brief Lists registered value IDs.
Result listValueIDs(EditorId id);

/// @brief Creates an element from serialized JSON.
Result createElement(
    EditorId id,
    const ElementPlacement& where,
    const std::string& json
);

/// @brief Removes an element subtree.
Result removeElement(
    EditorId id,
    ElementRef element
);

/// @brief Reorders an element within its parent.
Result reorderElement(
    EditorId id,
    ElementRef element,
    std::size_t index
);

/// @brief Moves an element within its face.
Result moveElement(
    EditorId id,
    ElementRef element,
    const ElementPlacement& where
);

/// @brief Replaces an element while preserving its handle.
Result replaceElement(
    EditorId id,
    ElementRef element,
    const std::string& json
);

//----------[ PROPERTIES ]----------//

/// @brief Sets a face property from JSON.
Result setFaceProperty(
    EditorId id,
    NodeId faceId,
    const std::string& path,
    const std::string& json
);

/// @brief Returns a face property as JSON.
Result getFaceProperty(
    EditorId id,
    NodeId faceId,
    const std::string& path
);

/// @brief Returns face property metadata.
Result getFacePropertiesMeta(
    EditorId id,
    NodeId faceId,
    const std::string& path = ""
);

/// @brief Sets an element property from JSON.
Result setElementProperty(
    EditorId id,
    ElementRef element,
    const std::string& path,
    const std::string& json
);

/// @brief Returns an element property as JSON.
Result getElementProperty(
    EditorId id,
    ElementRef element,
    const std::string& path
);

/// @brief Returns element property metadata.
Result getElementPropertiesMeta(
    EditorId id,
    ElementRef element,
    const std::string& path = ""
);

//----------[ CLIPBOARD ]----------//

/// @brief Returns the clipboard state.
ClipboardSummary getClipboardSummary(EditorId id);

/// @brief Clears the clipboard.
void clearClipboard(EditorId id);

/// @brief Copies a face to the clipboard.
Result copyFace(
    EditorId id,
    NodeId faceId
);

/// @brief Cuts a face to the clipboard.
Result cutFace(
    EditorId id,
    NodeId faceId
);

/// @brief Pastes a face from the clipboard.
Result pasteFace(
    EditorId id,
    FacePlacement where = FacePlacement{}
);

/// @brief Copies an element to the clipboard.
Result copyElement(
    EditorId id,
    ElementRef element
);

/// @brief Cuts an element to the clipboard.
Result cutElement(
    EditorId id,
    ElementRef element
);

/// @brief Pastes an element from the clipboard.
Result pasteElement(
    EditorId id,
    const ElementPlacement& where
);

/// @brief Replaces an element with the clipboard element.
Result pasteToReplaceElement(
    EditorId id,
    ElementRef element
);

//----------[ HISTORY ]----------//

/// @brief Undoes the preceding edit.
bool undo(EditorId id);

/// @brief Redoes the next edit.
bool redo(EditorId id);

/// @brief Moves the history cursor to an index.
bool jumpTo(
    EditorId id,
    std::size_t index
);

/// @brief Returns the current history index.
std::size_t historyIndex(EditorId id);

/// @brief Returns history command names.
Result getHistory(EditorId id);

/// @brief Returns whether an undo operation is available.
bool canUndo(EditorId id);

/// @brief Returns whether a redo operation is available.
bool canRedo(EditorId id);

} // namespace mg::editor
