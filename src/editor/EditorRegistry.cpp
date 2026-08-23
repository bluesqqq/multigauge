#include <multigauge/editor/EditorRegistry.h>

#include <multigauge/container/HandlePool.h>
#include <multigauge/editor/Editor.h>

namespace mg::editor {
    
namespace {
HandlePool<Editor, EditorId> editors;
}

EditorId create() { return editors.emplace(); }
bool destroy(EditorId id) { return editors.remove(id); }
bool exists(EditorId id) { return editors.get(id) != nullptr; }
Editor* find(EditorId id) { return editors.get(id); }

} // namespace mg::editor
