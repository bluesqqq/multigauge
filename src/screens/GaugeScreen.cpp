#include <multigauge/screens/GaugeScreen.h>

#include <multigauge/RuntimeContext.h>
#include <multigauge/io/Log.h>

#include <rapidjson/document.h>

namespace mg {

GaugeScreen::GaugeScreen(const char* gaugePath) {
    face = std::make_unique<GaugeFace>();

    if (gaugePath) {
        this->gaugePath = gaugePath;
    }
}

bool GaugeScreen::reload() {
    if (!context || gaugePath.empty()) {
        loaded = false;
        return false;
    }

    assets = std::make_unique<AssetManager>(FS(), context->getGraphicsContext());

    LOG_INFO("gauge", "Loading gauge: %s", gaugePath.c_str());

    rapidjson::Document doc;
    if (!assets->loadJson(gaugePath, doc)) {
        loaded = false;
        return false;
    }

    if (!doc.IsObject()) {
        LOG_ERROR("gauge", "Gauge document root is not an object: %s", gaugePath.c_str());
        loaded = false;
        return false;
    }

    assets->clearEmbeddedAssets();

    const auto root = doc.GetObject();

    auto assetsIt = root.FindMember("assets");
    if (assetsIt == root.MemberEnd() || !assetsIt->value.IsArray()) {
        LOG_ERROR("gauge", "Gauge document is missing array field 'assets': %s", gaugePath.c_str());
        loaded = false;
        return false;
    }

    const rapidjson::Value& assetsValue = assetsIt->value;
    if (!assets->loadDocumentAssets(assetsValue.GetArray())) {
        LOG_ERROR("gauge", "Failed to load embedded assets: %s", gaugePath.c_str());
        loaded = false;
        return false;
    }

    auto rootIt = root.FindMember("root");
    if (rootIt == root.MemberEnd() || !rootIt->value.IsObject()) {
        LOG_ERROR("gauge", "Gauge document is missing object field 'root': %s", gaugePath.c_str());
        loaded = false;
        return false;
    }

    face = std::make_unique<GaugeFace>();
    face->load(rootIt->value);

    if (!face->init(*assets)) {
        LOG_ERROR("gauge", "Gauge face initialization failed: %s", gaugePath.c_str());
        loaded = false;
        return false;
    }

    LOG_INFO("gauge", "Gauge ready: %s", gaugePath.c_str());
    loaded = true;
    return true;
}

bool GaugeScreen::load(const char* gaugePath) {
    this->gaugePath = gaugePath ? gaugePath : "";
    return reload();
}

void GaugeScreen::unload() {
    assets.reset();
    face = std::make_unique<GaugeFace>();
    loaded = false;
}

void GaugeScreen::onShow(RuntimeContext& ctx) {
    context = &ctx;
    if (!gaugePath.empty()) {
        reload();
    }
}

void GaugeScreen::onHide(RuntimeContext& ctx) {
    if (context == &ctx) {
        context = nullptr;
    }
}

void GaugeScreen::update(RuntimeContext& ctx, uint64_t deltaUs) {
    if (!loaded || !face || &ctx != context) return;
    face->update(static_cast<int>(deltaUs));
}

void GaugeScreen::draw(RuntimeContext& ctx, Graphics& g) {
    if (!loaded || !face || &ctx != context) return;
    face->layout(g);
    face->draw(g);
}

} // namespace mg
