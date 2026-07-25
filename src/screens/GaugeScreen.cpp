#include <multigauge/screens/GaugeScreen.h>

#include <multigauge/runtime/RuntimeContext.h>
#include <multigauge/io/Log.h>

namespace mg {

GaugeScreen::GaugeScreen() = default;

void GaugeScreen::setFace(std::unique_ptr<::mg::gauge::GaugeFace> newFace) {
    face = std::move(newFace);
}

void GaugeScreen::onShow(RuntimeContext& ctx) {
    if (face) face->init(ctx.getAssetManager(), ctx.getGraphicsContext());
}

void GaugeScreen::onHide(RuntimeContext& ctx) {}

void GaugeScreen::update(RuntimeContext& ctx, uint64_t deltaUs) {
    if (!face) return;
    face->update(static_cast<int>(deltaUs));
}

void GaugeScreen::draw(RuntimeContext& ctx, graphics::Graphics& g) {
    if (!face) return;
    face->layout(g);
    face->draw(g);
}

} // namespace mg
