#include <multigauge/screens/GaugeScreen.h>

#include <multigauge/runtime/RuntimeContext.h>
#include <multigauge/io/Log.h>

namespace mg {

GaugeScreen::GaugeScreen() = default;

void GaugeScreen::setFace(std::unique_ptr<::mg::gauge::GaugeFace> newFace, std::string newPackageId) {
    face = std::move(newFace);
    packageId = std::move(newPackageId);
}

void GaugeScreen::onShow(RuntimeContext& ctx) {
    if (face) face->init(packageId, ctx.getAssetManager(), ctx.getGraphicsContext());
}

void GaugeScreen::onHide(RuntimeContext& ctx) {}

void GaugeScreen::update(RuntimeContext& ctx, std::chrono::microseconds delta) {
    if (!face) return;
    face->update(delta);
}

void GaugeScreen::draw(RuntimeContext& ctx, graphics::Graphics& g) {
    if (!face) return;
    face->layout(g);
    face->draw(g);
}

} // namespace mg
