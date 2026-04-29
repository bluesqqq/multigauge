#pragma once

#include <multigauge/RuntimeContext.h>
#include <multigauge/editor/Api.h>
#include <yoga/Yoga.h>

#include <multigauge/io/FileSystem.h>
#include <multigauge/io/Time.h>
#include <multigauge/io/Logger.h>

namespace mg {

using ContextId = uint32_t;

//----------[ LIFETIME ]----------//

bool init(io::FileSystem& fs, io::Time& time, io::Logger* logger = nullptr);
void shutdown();
void frame();
YGConfigRef getYogaConfig();

//----------[ CONTEXT ]----------//

ContextId addContext(graphics::GraphicsContext& graphics);
bool removeContext(ContextId id);

//----------[ SCREEN ]----------//

bool setScreen(ContextId, std::unique_ptr<Screen> screen);
bool clearScreen(ContextId id);

bool setGaugeScreen(ContextId id, const std::string& json);
bool setGaugeScreenFromFile(ContextId id, const std::string& path);
bool setEditorScreen(ContextId id, editor::EditorId editorId, editor::Editor::Id faceId);

}
