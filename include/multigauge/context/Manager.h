#pragma once

#include <chrono>
#include <cstddef>
#include <memory>
#include <string>

#include <multigauge/container/GenerationalHandle.h>
#include <multigauge/container/HandlePool.h>
#include <multigauge/context/Context.h>

namespace mg {

// Forward declarations
class Screen;
namespace graphics { class GraphicsContext; class UserPalette; }
namespace io { class FileSystem; }

using ContextId = GenerationalHandle<struct ContextTag>;

namespace context {

/// @brief Owns graphics contexts, installed screens, and frame dispatch for one runtime.
class Manager {
public:
    //----------[ CTOR + DTOR ]----------//
    Manager(
        io::FileSystem& fs,
        std::string dataRoot,
        const graphics::UserPalette& palette
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

    /// @brief Returns a borrowed pointer to a context's active screen, if any.
    [[nodiscard]] Screen* getScreen(ContextId id);

    /// @brief Returns a borrowed pointer to a context's active screen, if any.
    [[nodiscard]] const Screen* getScreen(ContextId id) const;

    //----------[ LIFECYCLE ]----------//

    void frame(std::chrono::microseconds delta, std::chrono::microseconds elapsed);

private:
    io::FileSystem& fs_;
    std::string root_;
    const graphics::UserPalette& palette_;
    HandlePool<Context, ContextId> contexts_;
};

} // namespace context

} // namespace mg
