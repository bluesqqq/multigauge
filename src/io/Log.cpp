#include <multigauge/io/Log.h>

namespace {
    mg::io::Logger* g_logger = nullptr;
}

namespace mg::io {

Logger* getLogger() {
    return g_logger;
}

// internal setter (not public API)
void setLogger(Logger* logger) {
    g_logger = logger;
}

} // namespace mg::io
