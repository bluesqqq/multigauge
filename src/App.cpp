#include <multigauge/App.h>

#include <multigauge/HandlePool.h>
#include <multigauge/Platform.h>
#include <multigauge/screens/GaugeScreen.h>
#include <multigauge/io/Time.h>

namespace mg {

namespace {
    HandlePool<RuntimeContext> contexts;
    bool initialized = false;
    uint64_t lastUs = 0;
}

bool init(Platform& platform) {
    if (initialized) {
        return true;
    }

    setPlatform(platform);
    if (!initPlatform()) {
        return false;
    }

    contexts.clear();
    initialized = true;
    lastUs = TIME().getMicros();
    return true;
}

void shutdown() {
    contexts.clear();
    initialized = false;
    lastUs = 0;
}

void frame() {
    if (!initialized) return;

    const uint64_t nowUs = TIME().getMicros();
    const uint64_t deltaUs = nowUs - lastUs;
    lastUs = nowUs;

    for (auto& context : contexts) {
        context.frame(deltaUs);
    }
}

ContextId addContext(GraphicsContext& graphics) {
    RuntimeContext context(graphics);
    const ContextId id = contexts.add(std::move(context));

    if (auto* created = contexts.get(id)) {
        created->setId(id);
    }

    return id;
}

bool removeContext(ContextId id) {
    return contexts.remove(id);
}

RuntimeContext* getContext(ContextId id) {
    return contexts.get(id);
}

bool setScreen(ContextId id, std::unique_ptr<Screen> screen) {
    RuntimeContext* context = getContext(id);
    if (!context || !screen) return false;
    return context->setScreen(std::move(screen));
}

bool clearScreen(ContextId id) {
    RuntimeContext* context = getContext(id);
    if (!context) return false;

    context->clearScreen();
    return true;
}

bool showGauge(ContextId id, const char* gaugePath) {
    if (!gaugePath) return false;
    return setScreen(id, std::make_unique<GaugeScreen>(gaugePath));
}

} // namespace mg
