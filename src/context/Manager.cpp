#include <multigauge/context/Manager.h>

#include <multigauge/Config.h>
#if MG_BUILD_EDITOR
#include <multigauge/editor/Manager.h>
#include <multigauge/screens/EditorScreen.h>
#endif
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/graphics/UserPalette.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/package/Manager.h>
#include <multigauge/screens/GaugeScreen.h>
#include <multigauge/utils/Json.h>

namespace mg::context {

Manager::Manager(io::FileSystem& fs, std::string root, const graphics::UserPalette& palette, package::Manager& packages
#if MG_BUILD_EDITOR
                 , editor::Manager& editors
#endif
)
    : fs_(fs), root_(std::move(root)), palette_(palette), packages_(packages)
#if MG_BUILD_EDITOR
      , editors_(editors)
#endif
{}

Manager::~Manager() = default;

ContextId Manager::add(graphics::GraphicsContext& graphics) {
    return contexts_.emplace(graphics, fs_, root_, palette_);
}

bool Manager::remove(ContextId id) {
    return contexts_.remove(id);
}

bool Manager::has(ContextId id) const noexcept {
    return contexts_.exists(id);
}

std::size_t Manager::count() const noexcept {
    return contexts_.size();
}

bool Manager::setScreen(ContextId id, std::unique_ptr<Screen> screen) {
    auto* c = contexts_.get(id);
    return c && screen && c->setScreen(std::move(screen));
}

bool Manager::clearScreen(ContextId id) {
    auto* c = contexts_.get(id);
    if (!c) return false;

    c->clearScreen();
    return true;
}

bool Manager::hasScreen(ContextId id) const {
    const auto* c = contexts_.get(id);
    return c && c->getScreen();
}

namespace {

bool loadFace(json::Reader value, std::unique_ptr<gauge::GaugeFace>& face) {
    if (!value.isObject()) return false;

    auto next = std::make_unique<gauge::GaugeFace>();
    if (!next->load(value)) return false;

    face = std::move(next);
    return true;
}

} // namespace

bool Manager::setGaugeScreen(ContextId id, const std::string& json) {
    auto* c = contexts_.get(id);
    auto document = json::parse(json);
    std::unique_ptr<gauge::GaugeFace> face;
    if (!c || !document.valid() || !loadFace(document.root(), face)) return false;

    auto screen = std::make_unique<GaugeScreen>();
    screen->setFace(std::move(face));
    return c->setScreen(std::move(screen));
}

bool Manager::setGaugeScreen(ContextId id, const std::string& packageId, const std::string& faceId) { auto* c = contexts_.get(id); if (!c) return false; Result result = packages_.getFace(packageId, faceId); std::unique_ptr<gauge::GaugeFace> face; if (!result.ok || !loadFace(result.data.root(), face)) return false; auto screen = std::make_unique<GaugeScreen>(); screen->setFace(std::move(face), packageId); return c->setScreen(std::move(screen)); }

#if MG_BUILD_EDITOR
bool Manager::setEditorScreen(ContextId id, editor::EditorId editorId, editor::NodeId faceId) {
    auto* c = contexts_.get(id);
    if (!c || !editors_.isFace(editorId, faceId)) return false;
    return c->setScreen(std::make_unique<EditorScreen>(editors_, editorId, faceId));
}
#endif

void Manager::frame(std::chrono::microseconds delta, std::chrono::microseconds elapsed) {
    for (auto& c : contexts_) c.frame(delta, elapsed);
}

void Manager::clear() {
    contexts_.clear();
}

} // namespace mg::context
