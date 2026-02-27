#include <multigauge/App.h>

#include <multigauge/Platform.h>
#include <multigauge/io/Time.h>
#include <multigauge/io/Log.h>

#include <multigauge/AssetManager.h>
#include <multigauge/graphics/Graphics.h>
#include <multigauge/gauge/GaugeFace.h>

#include <rapidjson/document.h>
#include <memory>
#include <cmath>

namespace {
    std::unique_ptr<AssetManager> assets;
    std::unique_ptr<Graphics> g;
    std::unique_ptr<GaugeFace> face;

    uint32_t lastUs = 0;
    int t = 0;
}

namespace mg {

bool init(const char* gaugePath) {
    g = std::make_unique<Graphics>(&GFX());
    assets = std::make_unique<AssetManager>(FS(), GFX());

    rapidjson::Document doc;
    if (assets->loadJson(gaugePath, doc)) {
        face = std::make_unique<GaugeFace>(doc);
        LOG_INFO("gauge", "Loaded gaugeface: %s", gaugePath);
    } else {
        face = std::make_unique<GaugeFace>();
        LOG_WARN("gauge", "Failed to load gaugeface: %s", gaugePath);
    }

    face->initRecursive(*assets);

    lastUs = TIME().getMicros();
    return true;
}

void frame() {
    uint64_t nowUs = TIME().getMicros();
    uint64_t deltaUs = nowUs - lastUs;
    lastUs = nowUs;

    static uint64_t accUs = 0;
    static uint64_t frames = 0;

    accUs += deltaUs;
    frames++;

    if (accUs >= 1'000'000u) {
        float fps = (frames * 1'000'000.0f) / (float)accUs;
        LOG_INFO("perf", "fps=%.2f frames=%u window_us=%u",
                 fps, (unsigned)frames, (unsigned)accUs);
        accUs = 0;
        frames = 0;
    }

    auto screen = g->getScreenBounds().toFloat();

    g->clearColorCache();
    GFX().beginFrame();

    face->layoutRecursive(screen.width, screen.height);
    face->updateRecursive(deltaUs);
    face->drawRecursive(*g);

    GFX().endFrame();

    engineRPM.setValueBase(std::sin((float)nowUs / 1000000.0f) * 3000 + 4000);
    engineCoolantTemp.setValueBase(std::sin((float)nowUs / 5000000.0f) * 10 + 0);
}

}