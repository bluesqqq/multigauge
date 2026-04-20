#include <multigauge/App.h>
#include <multigauge/GaugeView.h>

#include <multigauge/Platform.h>
#include <multigauge/io/Time.h>

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
}

GaugeFace& getGaugeFace() {
    return primaryView ? primaryView->getGaugeFace() : fallbackFace;
}

} // namespace mg
