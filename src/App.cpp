#include <multigauge/App.h>

#include <multigauge/Platform.h>
#include <multigauge/io/Time.h>
#include <multigauge/io/Log.h>

#include <multigauge/AssetManager.h>
#include <multigauge/graphics/Graphics.h>

#include <rapidjson/document.h>

#include <algorithm>
#include <cmath>
#include <memory>
#include <vector>

namespace {
    std::vector<std::unique_ptr<GaugeView>> views;
    GaugeView* primaryView = nullptr;
    GaugeFace fallbackFace;

    uint64_t lastUs = 0;
}

GaugeView::GaugeView(GraphicsContext& context)
    : context(&context),
      graphics(std::make_unique<Graphics>(&context)),
      assets(std::make_unique<AssetManager>(FS(), context)) {}

GaugeView::~GaugeView() = default;

bool GaugeView::load(const char* gaugePath) {
    if (!gaugePath || !assets) return false;

    rapidjson::Document doc;
    if (!assets->loadJson(gaugePath, doc)) return false;

    face.load(doc);
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

namespace mg {

GaugeView& createView(GraphicsContext& context, const char* gaugePath) {
    auto view = std::make_unique<GaugeView>(context);
    if (gaugePath) {
        view->load(gaugePath);
    }

    GaugeView* rawView = view.get();
    views.push_back(std::move(view));

    if (!primaryView) {
        primaryView = rawView;
    }

    return *rawView;
}

bool removeView(GaugeView& view) {
    auto it = std::find_if(views.begin(), views.end(), [&](const std::unique_ptr<GaugeView>& candidate) {
        return candidate.get() == &view;
    });

    if (it == views.end()) {
        return false;
    }

    const bool removedPrimary = (primaryView == it->get());
    views.erase(it);

    if (removedPrimary) {
        primaryView = views.empty() ? nullptr : views.front().get();
    }

    return true;
}

void clearViews() {
    views.clear();
    primaryView = nullptr;
}

GaugeView* getPrimaryView() { return primaryView; }

bool init(const char* gaugePath) {
    clearViews();
    primaryView = &createView(GFX(), gaugePath);

    lastUs = TIME().getMicros();
    return true;
}

void frame() {
    if (views.empty()) return;

    const uint64_t nowUs = TIME().getMicros();
    const uint64_t deltaUs = nowUs - lastUs;
    lastUs = nowUs;

    for (const auto& view : views) {
        view->frame(deltaUs);
    }

    engineRPM.setValueBase(std::sin((float)nowUs / 1000000.0f) * 3000 + 4000);
    engineCoolantTemp.setValueBase(std::sin((float)nowUs / 5000000.0f) * 10 + 0);
}

GaugeFace& getGaugeFace() {
    return primaryView ? primaryView->getGaugeFace() : fallbackFace;
}

} // namespace mg
