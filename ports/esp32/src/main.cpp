#include <multigauge/App.h>

#include "platform/GraphicsContextLovyanGFX.h"
#include "platform/LittleFsFileSystem.h"
#include "platform/SerialLogger.h"
#include "platform/TimeArduino.h"
#include "WebServer.h"

#include <Arduino.h>
#include <string>

namespace mg {
    SerialLogger logger;
    LittleFsFileSystem fileSystem;
    TimeArduino clock;
    GraphicsContextLovyanGFX context;
}

void setup() {
    if (!mg::init(mg::fileSystem, mg::clock, {}, &mg::logger)) return;

    if (!mg::context.init()) return;

    const mg::ContextId contextId = mg::addContext(mg::context);
    if (contextId == mg::ContextId(0)) return;

    mgweb::start(contextId);
}

void loop() {
    mg::frame();
    mgweb::process();
}
