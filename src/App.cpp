#include <multigauge/App.h>

#include <multigauge/HandlePool.h>
#include <multigauge/screens/GaugeScreen.h>

namespace mg {

namespace {
    YGConfigRef createYogaConfig() {
        YGConfigRef config = YGConfigNew();
        YGConfigSetUseWebDefaults(config, false);
        return config;
    }

    HandlePool<RuntimeContext> contexts;
    bool initialized = false;
    uint64_t lastUs = 0;
    YGConfigRef yogaConfig = nullptr;

    FileSystem* g_fs = nullptr;
    Time* g_time = nullptr;
}

bool init(FileSystem& fs, Time& time, Logger* logger) {
    if (initialized) return true;

    g_fs = &fs;
    g_time = &time;

    mg::setLogger(logger);

    if (logger) {
        if (!logger->init()) return false;
    }

    if (!g_fs->init()) return false;

    contexts.clear();
    yogaConfig = createYogaConfig();
    initialized = true;

    lastUs = g_time->getMicros();

    return true;
}

void shutdown() {
    contexts.clear();
    if (yogaConfig) {
        YGConfigFree(yogaConfig);
        yogaConfig = nullptr;
    }
    initialized = false;
    lastUs = 0;
}

void frame() {
    if (!initialized) return;

    const uint64_t nowUs = g_time->getMicros();
    const uint64_t deltaUs = nowUs - lastUs;
    lastUs = nowUs;

    for (auto& context : contexts) context.frame(deltaUs);
}

YGConfigRef getYogaConfig() { return yogaConfig; }

ContextId addContext(GraphicsContext& graphics) { return contexts.emplace(graphics, *g_fs); }

bool removeContext(ContextId id) { return contexts.remove(id); }

RuntimeContext* getContext(ContextId id) { return contexts.get(id); }

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

} // namespace mg
