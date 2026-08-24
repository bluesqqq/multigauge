#include <multigauge/context/Manager.h>

#include <multigauge/container/HandlePool.h>
#include <multigauge/editor/Api.h>
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/graphics/UserPalette.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/runtime/PackageManager.h>
#include <multigauge/runtime/RuntimeContext.h>
#include <multigauge/screens/EditorScreen.h>
#include <multigauge/screens/GaugeScreen.h>
#include <multigauge/utils/Json.h>

namespace mg::context {
class Manager::State {
public:
    State(io::FileSystem& fs, std::string root, const graphics::UserPalette& palette, PackageManager& packages)
        : fs(fs), root(std::move(root)), palette(palette), packages(packages) {}
    io::FileSystem& fs;
    std::string root;
    const graphics::UserPalette& palette;
    PackageManager& packages;
    HandlePool<RuntimeContext, ContextId> contexts;
};

Manager::Manager(io::FileSystem& fs, std::string root, const graphics::UserPalette& palette, PackageManager& packages)
    : state_(std::make_unique<State>(fs, std::move(root), palette, packages)) {}
Manager::~Manager() = default;
ContextId Manager::add(graphics::GraphicsContext& graphics) { return state_->contexts.emplace(graphics, state_->fs, state_->root, state_->palette); }
bool Manager::remove(ContextId id) { return state_->contexts.remove(id); }
bool Manager::has(ContextId id) const noexcept { return state_->contexts.exists(id); }
std::size_t Manager::count() const noexcept { return state_->contexts.size(); }
bool Manager::setScreen(ContextId id, std::unique_ptr<Screen> screen) { auto* c = state_->contexts.get(id); return c && screen && c->setScreen(std::move(screen)); }
bool Manager::clearScreen(ContextId id) { auto* c = state_->contexts.get(id); if (!c) return false; c->clearScreen(); return true; }
bool Manager::hasScreen(ContextId id) const { const auto* c = state_->contexts.get(id); return c && c->getScreen(); }
namespace { bool loadFace(json::Reader value, std::unique_ptr<gauge::GaugeFace>& face) { if (!value.isObject()) return false; auto next = std::make_unique<gauge::GaugeFace>(); if (!next->load(value)) return false; face = std::move(next); return true; } }
bool Manager::setGaugeScreen(ContextId id, const std::string& json) { auto* c = state_->contexts.get(id); auto document = json::parse(json); std::unique_ptr<gauge::GaugeFace> face; if (!c || !document.valid() || !loadFace(document.root(), face)) return false; auto screen = std::make_unique<GaugeScreen>(); screen->setFace(std::move(face)); return c->setScreen(std::move(screen)); }
bool Manager::setGaugeScreen(ContextId id, const std::string& packageId, const std::string& faceId) { auto* c = state_->contexts.get(id); if (!c) return false; Result result = state_->packages.getFace(packageId, faceId); std::unique_ptr<gauge::GaugeFace> face; if (!result.ok || !loadFace(result.data.root(), face)) return false; auto screen = std::make_unique<GaugeScreen>(); screen->setFace(std::move(face), packageId); return c->setScreen(std::move(screen)); }
bool Manager::setEditorScreen(ContextId id, editor::EditorId editorId, editor::NodeId faceId) { auto* c = state_->contexts.get(id); if (!c || !editor::isFace(editorId, faceId)) return false; return c->setScreen(std::make_unique<EditorScreen>(editorId, faceId)); }
void Manager::frame(std::chrono::microseconds delta, std::chrono::microseconds elapsed) { for (auto& c : state_->contexts) c.frame(delta, elapsed); }
void Manager::clear() { state_->contexts.clear(); }
} // namespace mg::context
