#include <multigauge/runtime/RuntimeContext.h>

#include <multigauge/screens/Screen.h>

#include <utility>

namespace mg {

RuntimeContext::RuntimeContext(graphics::GraphicsContext& context, io::FileSystem& fs, const graphics::UserPalette& userPalette)
    : context(&context), graphics(context), assets(fs), userPalette(&userPalette) {}

RuntimeContext::~RuntimeContext() = default;

graphics::GraphicsContext& RuntimeContext::getGraphicsContext() { return *context; }

const graphics::GraphicsContext& RuntimeContext::getGraphicsContext() const { return *context; }

graphics::Graphics& RuntimeContext::getGraphics() { return graphics; }

const graphics::Graphics& RuntimeContext::getGraphics() const { return graphics; }

AssetManager& RuntimeContext::getAssetManager() { return assets; }

const AssetManager& RuntimeContext::getAssetManager() const { return assets; }

void RuntimeContext::setBackgroundColor(graphics::rgba color) { backgroundColor = color; }

graphics::rgba RuntimeContext::getBackgroundColor() const { return backgroundColor; }

void RuntimeContext::clearScreen() {
    if (screen) {
        screen->onHide(*this);
        screen.reset();
    }
}

bool RuntimeContext::setScreen(OwnedScreen newScreen) {
    if (!newScreen) return false;

    if (screen) screen->onHide(*this);

    screen = std::move(newScreen);
    screen->onShow(*this);
    return true;
}

Screen* RuntimeContext::getScreen() { return screen.get(); }

const Screen* RuntimeContext::getScreen() const { return screen.get(); }

void RuntimeContext::frame(std::chrono::microseconds delta, std::chrono::microseconds elapsed) {
    if (!context) return;
    context->beginFrame();

    if (screen) {
        screen->update(*this, delta);
        colorFrame.refresh(elapsed, *userPalette);
        graphics.beginFrame(colorFrame);
        screen->draw(*this, graphics);
    } else {
        colorFrame.refresh(elapsed, *userPalette);
        graphics.beginFrame(colorFrame);
        graphics.fillAll(backgroundColor);
    }

    graphics.endFrame();
    context->endFrame();
}

} // namespace mg
