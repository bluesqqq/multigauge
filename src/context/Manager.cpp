#include <multigauge/context/Manager.h>

#include <multigauge/graphics/UserPalette.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/screens/Screen.h>

namespace mg::context {

Manager::Manager(io::FileSystem& fs, std::string root, const graphics::UserPalette& palette)
    : fs_(fs), root_(std::move(root)), palette_(palette) {}

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

Screen* Manager::getScreen(ContextId id) {
    auto* context = contexts_.get(id);
    return context ? context->getScreen() : nullptr;
}

const Screen* Manager::getScreen(ContextId id) const {
    const auto* context = contexts_.get(id);
    return context ? context->getScreen() : nullptr;
}

void Manager::frame(std::chrono::microseconds delta, std::chrono::microseconds elapsed) {
    for (auto& c : contexts_) c.frame(delta, elapsed);
}

void Manager::clear() {
    contexts_.clear();
}

} // namespace mg::context
