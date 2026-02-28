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
    OwnedElement face;

    uint32_t lastUs = 0;
}

namespace mg {

bool init(const char* gaugePath) {
    g = std::make_unique<Graphics>(&GFX());
    assets = std::make_unique<AssetManager>(FS(), GFX());

    rapidjson::Document doc;
    if (assets->loadJson(gaugePath, doc)) {
        const rapidjson::Document& cdoc = doc;
        face = Element::fromJson(nullptr, cdoc.GetObject());
        LOG_INFO("gauge", "Loaded gaugeface: %s", gaugePath);
    } else {
        face = std::make_unique<GaugeFace>();
        LOG_WARN("gauge", "Failed to load gaugeface: %s", gaugePath);
    }

    if (face) face->initRecursive(*assets);

    lastUs = TIME().getMicros();
    return true;
}

void frame() {
    if (!face || !g) return;

    uint64_t nowUs = TIME().getMicros();
    uint64_t deltaUs = nowUs - lastUs;
    lastUs = nowUs;

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

Element* getFaceRoot() {
    return face.get();
}

} // namespace mg