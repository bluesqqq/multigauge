#pragma once

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>

#include <multigauge/container/GenerationalHandle.h>
#include <multigauge/editor/Types.h>

namespace mg {
class PackageManager;
class Screen;
namespace graphics { class GraphicsContext; class UserPalette; }
namespace io { class FileSystem; }

using ContextId = GenerationalHandle<struct ContextTag>;
}

namespace mg::context {

/// @brief Owns graphics contexts, installed screens, and frame dispatch for one runtime.
class Manager {
public:
    Manager(io::FileSystem& fs, std::string dataRoot, const graphics::UserPalette& palette,
            PackageManager& packages);
    ~Manager();

    [[nodiscard]] ContextId add(graphics::GraphicsContext& graphics);
    [[nodiscard]] bool remove(ContextId id);
    [[nodiscard]] bool has(ContextId id) const noexcept;
    [[nodiscard]] std::size_t count() const noexcept;

    [[nodiscard]] bool setScreen(ContextId id, std::unique_ptr<Screen> screen);
    [[nodiscard]] bool clearScreen(ContextId id);
    [[nodiscard]] bool hasScreen(ContextId id) const;
    [[nodiscard]] bool setGaugeScreen(ContextId id, const std::string& json);
    [[nodiscard]] bool setGaugeScreen(ContextId id, const std::string& packageId, const std::string& faceId);
    [[nodiscard]] bool setEditorScreen(ContextId id, editor::EditorId editorId, editor::NodeId faceId);

    void frame(std::chrono::microseconds delta, std::chrono::microseconds elapsed);
    void clear();

private:
    class State;
    std::unique_ptr<State> state_;
};

} // namespace mg::context
