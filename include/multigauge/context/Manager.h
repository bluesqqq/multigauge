#pragma once

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>

#include <multigauge/container/GenerationalHandle.h>
#include <multigauge/container/HandlePool.h>
#include <multigauge/Config.h>
#include <multigauge/context/Context.h>
#if MG_BUILD_EDITOR
#include <multigauge/editor/Types.h>
#endif

namespace mg {

// Forward declarations
namespace package { class Manager; }
class Screen;
namespace graphics { class GraphicsContext; class UserPalette; }
namespace io { class FileSystem; }
#if MG_BUILD_EDITOR
namespace editor { class Manager; }
#endif

using ContextId = GenerationalHandle<struct ContextTag>;

namespace context {

/// @brief Owns graphics contexts, installed screens, and frame dispatch for one runtime.
class Manager {
public:
    //----------[ CTOR + DTOR ]----------//
    Manager(
        io::FileSystem& fs,
        std::string dataRoot,
        const graphics::UserPalette& palette,
        package::Manager& packages
#if MG_BUILD_EDITOR
        , editor::Manager& editors
#endif
    );
    ~Manager();

    //----------[ REGISTRY ]----------//

    /// @brief Adds a context linked to a graphics context.
    /// @return A `ContextId` handle to the context.
    [[nodiscard]] ContextId add(graphics::GraphicsContext& graphics);

    /// @brief Removes a registered context.
    /// @returns `true` if context was found and removed, `false` if not.
    [[nodiscard]] bool remove(ContextId id);

    /// @brief Checks if a context exists.
    [[nodiscard]] bool has(ContextId id) const noexcept;

    /// @brief The number of contexts currently registered.
    [[nodiscard]] std::size_t count() const noexcept;

    /// @brief Clears all registered contexts.
    void clear();

    //----------[ SCREENS ]----------//

    /// @brief Sets the screen of a context.
    [[nodiscard]] bool setScreen(ContextId id, std::unique_ptr<Screen> screen);

    /// @brief Clears the screen of a context.
    [[nodiscard]] bool clearScreen(ContextId id);

    /// @brief Checks if a context currently has a screen.
    [[nodiscard]] bool hasScreen(ContextId id) const;

    /// @brief Sets a context to a gauge screen from json
    [[nodiscard]] bool setGaugeScreen(ContextId id, const std::string& json);

    /// @brief Sets a context to a gauge screen from package ID and face ID.
    [[nodiscard]] bool setGaugeScreen(ContextId id, const std::string& packageId, const std::string& faceId);

#if MG_BUILD_EDITOR
    /// @brief Sets a context to an editor screen from an editor ID and face ID.
    [[nodiscard]] bool setEditorScreen(ContextId id, editor::EditorId editorId, editor::NodeId faceId);
#endif

    //----------[ LIFECYCLE ]----------//

    void frame(std::chrono::microseconds delta, std::chrono::microseconds elapsed);

private:
    io::FileSystem& fs_;
    std::string root_;
    const graphics::UserPalette& palette_;
    package::Manager& packages_;
#if MG_BUILD_EDITOR
    editor::Manager& editors_;
#endif
    HandlePool<Context, ContextId> contexts_;
};

} // namespace context

} // namespace mg
