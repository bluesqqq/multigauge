#include <multigauge/App.h>

#include "platform/FileSystemEmscripten.h"
#include "platform/LoggerWebConsole.h"
#include "platform/TimeWeb.h"

#include <emscripten.h>
#include <cstdint>

namespace mg {
    LoggerWebConsole logger;
    FileSystemEmscripten fileSystem;
    TimeWeb time;
}

static uint64_t lastMs = 0;

static void tick() {
    uint64_t now = mg::time.getMillis();
    uint64_t dt = (lastMs == 0) ? 0 : (now - lastMs);
    lastMs = now;
    (void)dt;

    mg::frame();
}

int main() {
    mg::AppConfig config;
    config.dataRoot = "/work";

    if (!mg::init(mg::fileSystem, mg::time, config, &mg::logger)) return 1;

    emscripten_set_main_loop(tick, 0, 1);
    return 0;
}
