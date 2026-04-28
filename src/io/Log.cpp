#include <multigauge/io/Log.h>

namespace {
    Logger* g_logger = nullptr;
}

namespace mg {

Logger* getLogger() {
    return g_logger;
}

// internal setter (not public API)
void setLogger(Logger* logger) {
    g_logger = logger;
}

}