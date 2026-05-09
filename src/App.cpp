#include <multigauge/App.h>

#include <multigauge/RuntimeContext.h>
#include <multigauge/HandlePool.h>
#include <multigauge/editor/Api.h>
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/screens/GaugeScreen.h>
#include <multigauge/screens/EditorScreen.h>
#include <multigauge/io/Log.h>

#include <rapidjson/document.h>

#include <utility>

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

    io::FileSystem* g_fs = nullptr;
    io::Time* g_time = nullptr;
}

bool init(io::FileSystem& fs, io::Time& time, io::Logger* logger) {
    if (initialized) return true;

    g_fs = &fs;
    g_time = &time;

    io::setLogger(logger);

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

ContextId addContext(graphics::GraphicsContext& graphics) { return contexts.emplace(graphics, *g_fs); }

bool removeContext(ContextId id) { return contexts.remove(id); }

RuntimeContext* getContext(ContextId id) { return contexts.get(id); }

bool hasContext(ContextId id) { return contexts.exists(id); }

std::size_t contextCount() { return contexts.size(); }

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

bool hasScreen(ContextId id) {
    RuntimeContext* context = getContext(id);
    if (!context) return false;

    return context->getScreen();
}

namespace {
bool loadGaugeFaceFromJson(const std::string& json, std::unique_ptr<gauge::GaugeFace>& outFace) {
    rapidjson::Document doc;
    doc.Parse(json.c_str());
    if (doc.HasParseError() || !doc.IsObject()) return false;

    auto face = std::make_unique<gauge::GaugeFace>();
    face->load(doc);
    outFace = std::move(face);
    return true;
}
}

bool setGaugeScreen(ContextId id, const std::string& json) {
    RuntimeContext* context = getContext(id);
    if (!context) return false;

    std::unique_ptr<gauge::GaugeFace> face;
    if (!loadGaugeFaceFromJson(json, face)) return false;

    auto screen = std::make_unique<GaugeScreen>();
    screen->setFace(std::move(face));
    return context->setScreen(std::move(screen));
}

bool setGaugeScreenFromFile(ContextId id, const std::string& path) {
    if (!g_fs) return false;

    std::string json;
    if (!g_fs->readText(path, json)) return false;

    return setGaugeScreen(id, json);
}

bool setEditorScreen(ContextId id, editor::EditorId editorId, editor::NodeId faceId) {
    RuntimeContext* context = getContext(id);
    if (!context) return false;
    if (!editor::exists(editorId) || !editor::isFace(editorId, faceId)) return false;

    auto screen = std::make_unique<EditorScreen>(editorId, faceId);
    return context->setScreen(std::move(screen));
}

} // namespace mg
