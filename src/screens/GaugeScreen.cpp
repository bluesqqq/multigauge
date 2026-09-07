#include <multigauge/screens/GaugeScreen.h>

#include <multigauge/context/Context.h>
#include <multigauge/io/Log.h>

namespace mg {

GaugeScreen::GaugeScreen() = default;

void GaugeScreen::setFace(std::unique_ptr<::mg::gauge::GaugeFace> newFace, std::string newPackageId) {
    face = std::move(newFace);
    packageId = std::move(newPackageId);
}

void GaugeScreen::onShow(context::Context& ctx) {
    if (face) face->init(packageId, ctx.getAssetManager(), ctx.getGraphicsContext());
}

void GaugeScreen::onHide(context::Context& ctx) {}

void GaugeScreen::update(context::Context& ctx, std::chrono::microseconds delta) {
    if (!face) return;
    face->update(delta);
}

void GaugeScreen::draw(context::Context& ctx, graphics::Graphics& g) {
    if (!face) return;
    face->layout(g);
    face->draw(g);
}

} // namespace mg
