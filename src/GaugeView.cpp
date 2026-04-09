#include <multigauge/GaugeView.h>

#include <multigauge/Platform.h>
#include <multigauge/io/Log.h>

#include <multigauge/AssetManager.h>
#include <multigauge/graphics/Graphics.h>

#include <rapidjson/document.h>

#include <memory>

GaugeView::GaugeView(GraphicsContext& context)
    : context(&context),
      graphics(std::make_unique<Graphics>(&context)),
      assets(std::make_unique<AssetManager>(FS(), context)) {}

GaugeView::~GaugeView() = default;

bool GaugeView::load(const char* gaugePath) {
    if (!gaugePath || !assets) return false;

    rapidjson::Document doc;
    if (!assets->loadJson(gaugePath, doc)) return false;
    if (!doc.IsObject()) return false;

    assets->clearEmbeddedAssets();

    const auto root = doc.GetObject();

    auto assetsIt = root.FindMember("assets");
    if (assetsIt == root.MemberEnd() || !assetsIt->value.IsArray()) {
        LOG_ERROR("gauge", "Gauge document is missing array field 'assets': %s", gaugePath);
        return false;
    }

    const auto& assetsValue = assetsIt->value;
    if (!assets->loadDocumentAssets(assetsValue.GetArray())) {
        LOG_ERROR("gauge", "Failed to load embedded assets: %s", gaugePath);
        return false;
    }

    auto rootIt = root.FindMember("root");
    if (rootIt == root.MemberEnd() || !rootIt->value.IsObject()) {
        LOG_ERROR("gauge", "Gauge document is missing object field 'root': %s", gaugePath);
        return false;
    }

    face.load(rootIt->value);
    LOG_INFO("gauge", "Loaded gaugeface: %s", gaugePath);

    return face.init(*assets);
}

void GaugeView::frame(uint64_t deltaUs) {
    if (!context || !graphics) return;

    graphics->clearColorCache();
    context->beginFrame();

    face.layout(*graphics);
    face.update(static_cast<int>(deltaUs));
    face.draw(*graphics);

    context->endFrame();
}

GraphicsContext& GaugeView::getContext() { return *context; }

const GraphicsContext& GaugeView::getContext() const { return *context; }

Graphics& GaugeView::getGraphics() { return *graphics; }

const Graphics& GaugeView::getGraphics() const { return *graphics; }

GaugeFace& GaugeView::getGaugeFace() { return face; }

const GaugeFace& GaugeView::getGaugeFace() const { return face; }
