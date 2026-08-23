#pragma once

#include <multigauge/editor/Types.h>

namespace mg::editor {
class Editor;

/// Owns live editor instances addressed by stable editor IDs.
/// @brief Creates an editor instance.
EditorId create();
/// @brief Destroys an editor instance.
bool destroy(EditorId id);
/// @brief Tests whether an editor instance is live.
bool exists(EditorId id);
/// @brief Returns a borrowed editor instance, or null when `id` is not live.
Editor* find(EditorId id);

} // namespace mg::editor
