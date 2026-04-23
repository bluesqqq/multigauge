#pragma once

#include <multigauge/Platform.h>
#include <multigauge/RuntimeContext.h>

namespace mg {

using ContextId = uint32_t;

//----------[ LIFETIME ]----------//

bool init(Platform& platform);
void shutdown();
void frame();

//----------[ CONTEXT ]----------//

ContextId addContext(GraphicsContext& graphics);
bool removeContext(ContextId id);
RuntimeContext* getContext(ContextId id);

//----------[ SCREEN ]----------//

bool setScreen(ContextId, std::unique_ptr<Screen> screen);
bool clearScreen(ContextId id);
bool showGauge(ContextId id, const char* gaugePath);

}
