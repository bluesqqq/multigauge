#pragma once

#include <cstdint>
#include <memory>

#include <multigauge/editor/Api.h>
#include <yoga/Yoga.h>

#include <multigauge/io/FileSystem.h>
#include <multigauge/io/Time.h>
#include <multigauge/io/Logger.h>

namespace mg {

class Screen;

namespace graphics {
class GraphicsContext;
}

using ContextId = uint32_t;

//----------[ LIFETIME ]----------//

bool init(io::FileSystem& fs, io::Time& time, io::Logger* logger = nullptr, const std::string& dataRoot = "/multigauge");
void shutdown();
void frame();
YGConfigRef getYogaConfig();

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
bool setGaugeScreenFromFile(ContextId id, const std::string& path);
bool setEditorScreen(ContextId id, editor::EditorId editorId, editor::NodeId faceId);

Result getGaugeLibrary();

}
