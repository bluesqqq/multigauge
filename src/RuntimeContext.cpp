#include <multigauge/RuntimeContext.h>

#include <multigauge/screens/Screen.h>

#include <utility>

namespace mg {

RuntimeContext::RuntimeContext(graphics::GraphicsContext& context, io::FileSystem& fs) : context(&context), graphics(context), assets(fs) {}

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

void RuntimeContext::frame(uint64_t deltaUs) {
    if (!context) return;

    graphics.clearColorCache();

    // TODO: maybe allow graphics to handle begin/end so i dont have to do this?
    context->beginFrame();

    if (screen) {
        screen->update(*this, deltaUs);
        screen->draw(*this, graphics);
    } else {
        graphics.fillAll(backgroundColor);
    }

    context->endFrame();
}

} // namespace mg
