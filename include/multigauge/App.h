#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <multigauge/json/Json.h>

#include <multigauge/editor/Api.h>
#include <multigauge/runtime/PackageManager.h>
#include <yoga/Yoga.h>

#include <multigauge/io/FileSystem.h>
#include <multigauge/io/Time.h>
#include <multigauge/io/Logger.h>
#include <multigauge/graphics/UserPalette.h>

namespace mg {

class Screen;

namespace graphics {
class GraphicsContext;
}

using ContextId = uint32_t;

struct AppConfig {
    std::string dataRoot = "/multigauge";
};

//----------[ LIFETIME ]----------//

bool init(io::FileSystem& fs, io::Time& time, const AppConfig& config = AppConfig{}, io::Logger* logger = nullptr);
void shutdown();
void frame();
YGConfigRef getYogaConfig();

//----------[ USER PALETTE ]----------//

bool setUserColor(std::size_t slot, graphics::rgba color);
graphics::rgba getUserColor(std::size_t slot);

//----------[ CONTEXT ]----------//

ContextId addContext(graphics::GraphicsContext& graphics);
bool removeContext(ContextId id);
bool hasContext(ContextId id);
std::size_t contextCount();

//----------[ SCREEN ]----------//

bool setScreen(ContextId, std::unique_ptr<Screen> screen);
bool clearScreen(ContextId id);
bool hasScreen(ContextId id);

bool setGaugeScreen(ContextId id, const std::string& json);
bool setGaugeScreen(ContextId id, const std::string& packageId, const std::string& faceId);
bool setEditorScreen(ContextId id, editor::EditorId editorId, editor::NodeId faceId);

//----------[ PACKAGES ]----------//

bool listPackages(std::vector<PackageSummary>& out);
bool listFaces(const std::string& packageId, std::vector<FaceSummary>& out);

Result getPackage(const std::string& packageId);
Result importPackage(const std::string& json);
Result importPackage(json::Reader package);
Result exportPackage(const std::string& packageId);
Result removePackage(const std::string& packageId);

}
