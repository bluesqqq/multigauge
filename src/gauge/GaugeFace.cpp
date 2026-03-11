#include <multigauge/gauge/GaugeFace.h>

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

