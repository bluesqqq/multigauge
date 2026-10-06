#include <multigauge/screens/GaugeScreen.h>

#include <multigauge/control/Action.h>
#include <multigauge/context/Context.h>
#include <multigauge/io/Log.h>

namespace mg {

GaugeScreen::GaugeScreen() = default;

void GaugeScreen::setFace(
    std::unique_ptr<::mg::gauge::GaugeFace> newFace,
    std::string newPackageId,
    std::string newFaceId
) {
    face = std::move(newFace);
    packageId_ = std::move(newPackageId);
    faceId_ = std::move(newFaceId);
}

const std::string& GaugeScreen::packageId() const noexcept { return packageId_; }

const std::string& GaugeScreen::faceId() const noexcept { return faceId_; }

void GaugeScreen::onShow(context::Context& ctx) {
    if (face) face->init(packageId_, ctx.getAssetManager(), ctx.getGraphicsContext());
}

void GaugeScreen::onHide(context::Context& ctx) {}

control::Result GaugeScreen::onControl(control::Action action) {
    switch (action) {
        case control::Action::Next:
            return control::Result::nextFace();
        case control::Action::Previous:
            return control::Result::previousFace();
        case control::Action::Select:
            return control::Result::openMenu();
    }
    return control::Result::ignored();
}

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
