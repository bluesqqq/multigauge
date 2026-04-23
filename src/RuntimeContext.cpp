#include <multigauge/RuntimeContext.h>

#include <multigauge/screens/Screen.h>

namespace mg {

RuntimeContext::RuntimeContext(GraphicsContext& graphicsContext)
    : native(&graphicsContext), graphics(&graphicsContext) {}

RuntimeContext::~RuntimeContext() = default;

void RuntimeContext::setId(ContextId contextId) { id = contextId; }

ContextId RuntimeContext::getId() const { return id; }

GraphicsContext& RuntimeContext::getGraphicsContext() { return *native; }

const GraphicsContext& RuntimeContext::getGraphicsContext() const { return *native; }

Graphics& RuntimeContext::getGraphics() { return graphics; }

const Graphics& RuntimeContext::getGraphics() const { return graphics; }

void RuntimeContext::setBackgroundColor(rgba color) { backgroundColor = color; }

rgba RuntimeContext::getBackgroundColor() const { return backgroundColor; }

void RuntimeContext::clearScreen() {
    if (activeScreen) {
        activeScreen->onHide(*this);
        activeScreen.reset();
    }
}

bool RuntimeContext::setScreen(OwnedScreen screen) {
    if (!screen) return false;

    if (activeScreen) {
        activeScreen->onHide(*this);
    }

    activeScreen = std::move(screen);
    activeScreen->onShow(*this);
    return true;
}

Screen* RuntimeContext::getScreen() { return activeScreen.get(); }

const Screen* RuntimeContext::getScreen() const { return activeScreen.get(); }

void RuntimeContext::frame(uint64_t deltaUs) {
    if (!native) return;

    graphics.clearColorCache();
    native->beginFrame();
    graphics.fillAll(backgroundColor);

    if (activeScreen) {
        activeScreen->update(*this, deltaUs);
        activeScreen->draw(*this, graphics);
    }

    native->endFrame();
}

} // namespace mg
