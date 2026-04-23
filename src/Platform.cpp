#include <multigauge/Platform.h>

#include <multigauge/graphics/GraphicsContext.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/io/Time.h>
#include <multigauge/io/Logger.h>

#include <cstdlib>
#include <cstdio>

namespace {
    Platform* g_platform = nullptr;

    [[noreturn]] void panic() {
        std::fputs(
            "multigauge-core: Platform not initialized.\n"
            "Call setPlatform(platform) once at startup before using GFX()/FS()/TIME()/LOG() or any core API.\n",
            stderr
        );
        std::abort();
    }
}

Platform& platform() {
    if (!g_platform) panic();
    return *g_platform;
}

void setPlatform(Platform& p) {
    if (g_platform && g_platform != &p) {
        std::fputs("multigauge-core: setPlatform() called more than once.\n", stderr);
        std::abort();
    }
    g_platform = &p;
}

bool initPlatform() {
    auto& p = platform();

    if (p.logger) {
        if (!p.logger->init()) return false;
    }

    if (!p.fs.init()) return false;

    return true;
}