#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>

#include <multigauge/graphics/UserPalette.h>
#include <multigauge/json/Json.h>

namespace mg::io { class FileSystem; }

namespace mg::settings {

/// @brief The installed gauge face selected for display at the next startup.
struct ActiveFace {
    std::string packageId;
    std::string faceId;
};

/// @brief Stores durable device preferences that are independent of gauge packages.
/// @details This intentionally contains only user preferences, not sensor configuration,
/// live readings, render state, or rebuildable package data.
class Manager {
public:
    Manager(io::FileSystem& fs, std::string dataRoot);

    /// @brief Loads settings from `settings.json` and applies saved colors to `palette`.
    /// @return True when settings were loaded or no settings file exists.
    [[nodiscard]] bool load(graphics::UserPalette& palette);

    /// @brief Writes changed settings atomically to `settings.json`.
    [[nodiscard]] bool save();

    /// @brief Returns the saved face selection, if one has been made.
    [[nodiscard]] const std::optional<ActiveFace>& activeFace() const noexcept;

    /// @brief Replaces the saved face selection.
    void setActiveFace(std::string packageId, std::string faceId);

    /// @brief Removes the saved face selection.
    void clearActiveFace() noexcept;

    /// @brief Returns a saved shared palette color.
    [[nodiscard]] graphics::rgba userColor(std::size_t slot) const noexcept;

    /// @brief Replaces one saved shared palette color.
    [[nodiscard]] bool setUserColor(std::size_t slot, graphics::rgba color) noexcept;

private:
    [[nodiscard]] bool loadDocument(json::Reader root, graphics::UserPalette& palette);
    [[nodiscard]] bool writeDocument(json::Writer& writer) const;

    io::FileSystem& fs_;
    std::string dataRoot_;
    std::optional<ActiveFace> activeFace_;
    std::array<graphics::rgba, graphics::UserPalette::Size> userColors_{};
    bool dirty_ = false;
};

} // namespace mg::settings
