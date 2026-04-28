#include <multigauge/screens/GaugeScreen.h>

#include <multigauge/RuntimeContext.h>
#include <multigauge/io/Log.h>

#include <rapidjson/document.h>

namespace mg {

GaugeScreen::GaugeScreen(GaugeFace* face) : face(face) {}

void GaugeScreen::setFace(GaugeFace* f) { face = f;}

void GaugeScreen::onShow(RuntimeContext& ctx) {
    context = &ctx;
}

void GaugeScreen::onHide(RuntimeContext& ctx) {
    if (context == &ctx) context = nullptr;
}

void GaugeScreen::update(RuntimeContext& ctx, uint64_t deltaUs) {
    if (!face || &ctx != context) return;
    face->update(static_cast<int>(deltaUs));
}

void GaugeScreen::draw(RuntimeContext& ctx, Graphics& g) {
    if (!face || &ctx != context) return;
    face->layout(g);
    face->draw(g);
}

} // namespace mg
