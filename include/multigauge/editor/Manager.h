#pragma once

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

    [[nodiscard]] ClipboardState& clipboard() noexcept { return clipboard_; }
    [[nodiscard]] const ClipboardState& clipboard() const noexcept { return clipboard_; }

private:
    HandlePool<Editor, EditorId> editors_;
    ClipboardState clipboard_;
};

} // namespace mg::editor
