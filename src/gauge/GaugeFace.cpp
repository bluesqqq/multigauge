#include <multigauge/gauge/GaugeFace.h>

namespace {
    YGConfigRef createGaugeConfig() {
        YGConfigRef config = YGConfigNew();
        YGConfigSetUseWebDefaults(config, false);
        return config;
    }
}

GaugeFace::GaugeFace() : config(createGaugeConfig(), &YGConfigFree), root(nullptr, config.get()) {}

void GaugeFace::load(const rapidjson::Document& doc) {
    if (!doc.IsObject()) return;
    root.loadFromJson(doc.GetObject());
}

rapidjson::Document GaugeFace::save() const {
    rapidjson::Document doc;
    auto& a = doc.GetAllocator();
    root.saveToJson(doc, a);
    return doc;
}

void GaugeFace::layout(Graphics &g) {
    auto screen = g.getScreenBounds().toFloat();
    root.layoutRecursive(screen.width, screen.height);
}

void GaugeFace::draw(Graphics &g) const { root.drawRecursive(g); }

void GaugeFace::update(int deltaTime) { root.updateRecursive(deltaTime); }

bool GaugeFace::init(AssetManager &assetManager) { return root.initRecursive(assetManager); }

