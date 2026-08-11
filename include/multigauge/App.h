#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <multigauge/container/GenerationalHandle.h>
#include <multigauge/json/Json.h>

#include <multigauge/editor/Api.h>
#include <multigauge/runtime/PackageManager.h>

#include <multigauge/graphics/UserPalette.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/io/Logger.h>
#include <multigauge/io/Time.h>

namespace mg {

class Screen;

namespace graphics {
class GraphicsContext;
}

using ContextId = GenerationalHandle<struct ContextTag>;

/// @brief Configures core application initialization.
struct AppConfig {
    std::string dataRoot = "/multigauge";
};

//----------[ LIFETIME ]----------//

/// @brief Initializes the multigauge core runtime.
bool init(
    io::FileSystem& fs,
    io::Time& time,
    const AppConfig& config = AppConfig{},
    io::Logger* logger = nullptr
);

/// @brief Shuts down the multigauge core runtime.
void shutdown();

/// @brief Advances the core runtime by one frame.
void frame();

//----------[ USER PALETTE ]----------//

/// @brief Sets one user palette color.
bool setUserColor(std::size_t slot, graphics::rgba color);

/// @brief Returns one user palette color.
graphics::rgba getUserColor(std::size_t slot);

//----------[ CONTEXT ]----------//

/// @brief Registers a graphics context and returns its handle.
ContextId addContext(graphics::GraphicsContext& graphics);

/// @brief Removes a graphics context.
bool removeContext(ContextId id);

/// @brief Returns whether a graphics context is registered.
bool hasContext(ContextId id);

/// @brief Returns the number of registered graphics contexts.
std::size_t contextCount();

//----------[ SCREEN ]----------//

/// @brief Assigns a screen to a graphics context.
bool setScreen(ContextId, std::unique_ptr<Screen> screen);

/// @brief Removes the screen assigned to a graphics context.
bool clearScreen(ContextId id);

/// @brief Returns whether a graphics context has an assigned screen.
bool hasScreen(ContextId id);

/// @brief Assigns a gauge screen decoded from JSON.
bool setGaugeScreen(ContextId id, const std::string& json);

/// @brief Assigns a gauge screen from a stored package face.
bool setGaugeScreen(
    ContextId id,
    const std::string& packageId,
    const std::string& faceId
);

/// @brief Assigns an editor screen for an editor-owned face.
bool setEditorScreen(
    ContextId id,
    editor::EditorId editorId,
    editor::NodeId faceId
);

//----------[ PACKAGES ]----------//

/// @brief Lists stored package summaries.
bool listPackages(std::vector<PackageSummary>& out);

/// @brief Lists faces contained in a stored package.
bool listFaces(const std::string& packageId, std::vector<FaceSummary>& out);

/// @brief Retrieves a stored package document.
Result getPackage(const std::string& packageId);

/// @brief Imports a package document from JSON.
Result importPackage(const std::string& json);

/// @brief Imports a package document from a JSON reader.
Result importPackage(json::Reader package);

/// @brief Exports a stored package document.
Result exportPackage(const std::string& packageId);

/// @brief Removes a stored package.
Result removePackage(const std::string& packageId);

} // namespace mg
