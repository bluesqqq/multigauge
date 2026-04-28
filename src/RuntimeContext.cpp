#include <multigauge/RuntimeContext.h>

#include <multigauge/screens/Screen.h>

namespace mg {

RuntimeContext::RuntimeContext(GraphicsContext& context, FileSystem& fs) : context(&context), graphics(context), assets(fs) {}

RuntimeContext::~RuntimeContext() = default;

GraphicsContext& RuntimeContext::getGraphicsContext() { return *context; }

const GraphicsContext& RuntimeContext::getGraphicsContext() const { return *context; }

Graphics& RuntimeContext::getGraphics() { return graphics; }

const Graphics& RuntimeContext::getGraphics() const { return graphics; }

void RuntimeContext::setBackgroundColor(rgba color) { backgroundColor = color; }

rgba RuntimeContext::getBackgroundColor() const { return backgroundColor; }

void RuntimeContext::clearScreen() {
    if (screen) {
        screen->onHide(*this);
        screen.reset();
    }
}

bool RuntimeContext::setScreen(OwnedScreen screen) {
    if (!screen) return false;

    if (screen) screen->onHide(*this);

    screen = std::move(screen);
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
