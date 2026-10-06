#include <multigauge/navigation/Manager.h>

#if MG_BUILD_EDITOR
#include <multigauge/editor/Manager.h>
#include <multigauge/screens/EditorScreen.h>
#endif
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/package/Manager.h>
#include <multigauge/screens/GaugeScreen.h>
#include <multigauge/screens/Screen.h>
#include <multigauge/utils/Json.h>

#include <memory>

namespace mg::navigation {

Manager::Manager(
    context::Manager& contexts,
    package::Manager& packages
#if MG_BUILD_EDITOR
    , editor::Manager& editors
#endif
) : contexts_(contexts),
    packages_(packages)
#if MG_BUILD_EDITOR
    , editors_(editors)
#endif
{}

namespace {

bool loadFace(json::Reader value, std::unique_ptr<gauge::GaugeFace>& face) {
    if (!value.isObject()) return false;

    auto next = std::make_unique<gauge::GaugeFace>();
    if (!next->load(value)) return false;

    face = std::move(next);
    return true;
}

} // namespace

bool Manager::showGauge(ContextId id, const std::string& json) {
    auto document = json::parse(json);
    std::unique_ptr<gauge::GaugeFace> face;
    if (!document.valid() || !loadFace(document.root(), face)) return false;

    auto screen = std::make_unique<GaugeScreen>();
    screen->setFace(std::move(face));
    return contexts_.setScreen(id, std::move(screen));
}

bool Manager::showGauge(ContextId id, std::string_view packageId, std::string_view faceId) {
    const std::string packageIdCopy(packageId);
    const std::string faceIdCopy(faceId);
    Result result = packages_.getFace(packageIdCopy, faceIdCopy);
    std::unique_ptr<gauge::GaugeFace> face;
    if (!result.ok || !loadFace(result.data.root(), face)) return false;

    auto screen = std::make_unique<GaugeScreen>();
    screen->setFace(std::move(face), packageIdCopy, faceIdCopy);
    return contexts_.setScreen(id, std::move(screen));
}

#if MG_BUILD_EDITOR
bool Manager::showEditor(ContextId id, editor::EditorId editorId, editor::NodeId faceId) {
    if (!editors_.isFace(editorId, faceId)) return false;
    return contexts_.setScreen(id, std::make_unique<EditorScreen>(editors_, editorId, faceId));
}
#endif

control::Result Manager::handleControl(ContextId id, const control::Action& action) {
    Screen* screen = contexts_.getScreen(id);
    if (!screen) return control::Result::ignored();

    const control::Result result = screen->onControl(action);
    if (result.type == control::Result::Type::NextFace && !cycleGaugeFace(id, 1)) {
        return control::Result::ignored();
    }
    if (result.type == control::Result::Type::PreviousFace && !cycleGaugeFace(id, -1)) {
        return control::Result::ignored();
    }

    return result;
}

bool Manager::cycleGaugeFace(ContextId id, int offset) {
    const auto* screen = dynamic_cast<const GaugeScreen*>(contexts_.getScreen(id));
    if (!screen || screen->packageId().empty() || screen->faceId().empty()) return false;

    FaceSummary next;
    return packages_.offsetFace(screen->packageId(), screen->faceId(), offset, next) &&
           showGauge(id, screen->packageId(), next.id);
}

} // namespace mg::navigation
