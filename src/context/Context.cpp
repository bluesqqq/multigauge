#include <multigauge/context/Context.h>

#include <multigauge/screens/Screen.h>

#include <utility>

namespace mg::context {

Context::Context(
    graphics::GraphicsContext& graphicsContext,
    io::FileSystem& fs,
    std::string dataRoot,
    const graphics::UserPalette& userPalette
) : graphicsContext_(&graphicsContext),
    graphics_(graphicsContext),
    assets_(fs, std::move(dataRoot)),
    userPalette_(&userPalette) {}

Context::~Context() = default;

graphics::GraphicsContext& Context::getGraphicsContext() { return *graphicsContext_; }

const graphics::GraphicsContext& Context::getGraphicsContext() const { return *graphicsContext_; }

graphics::Graphics& Context::getGraphics() { return graphics_; }

const graphics::Graphics& Context::getGraphics() const { return graphics_; }

AssetManager& Context::getAssetManager() { return assets_; }

const AssetManager& Context::getAssetManager() const { return assets_; }

void Context::setBackgroundColor(graphics::rgba color) { backgroundColor_ = color; }

graphics::rgba Context::getBackgroundColor() const { return backgroundColor_; }

void Context::clearScreen() {
    if (!screen_) return;

    screen_->onHide(*this);
    screen_.reset();
}

bool Context::setScreen(OwnedScreen screen) {
    if (!screen) return false;
    if (screen_) screen_->onHide(*this);

    screen_ = std::move(screen);
    screen_->onShow(*this);
    return true;
}

Screen* Context::getScreen() { return screen_.get(); }

const Screen* Context::getScreen() const { return screen_.get(); }

void Context::frame(std::chrono::microseconds delta, std::chrono::microseconds elapsed) {
    if (!graphicsContext_) return;

    graphicsContext_->beginFrame();

    if (screen_) {
        screen_->update(*this, delta);
        colorFrame_.refresh(elapsed, *userPalette_);
        graphics_.beginFrame(colorFrame_);
        screen_->draw(*this, graphics_);
    } else {
        colorFrame_.refresh(elapsed, *userPalette_);
        graphics_.beginFrame(colorFrame_);
        graphics_.fillAll(backgroundColor_);
    }

    graphics_.endFrame();
    graphicsContext_->endFrame();
}

} // namespace mg::context
