#include <multigauge/App.h>

#include "AppPaths.h"

#include <multigauge/runtime/PackageManager.h>

#include <multigauge/runtime/RuntimeContext.h>
#include <multigauge/HandlePool.h>
#include <multigauge/editor/Api.h>
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/graphics/colors/TimeColor.h>
#include <multigauge/screens/GaugeScreen.h>
#include <multigauge/screens/EditorScreen.h>
#include <multigauge/io/Log.h>
#include <multigauge/utils/Json.h>

#include <utility>
#include <chrono>

namespace mg {

namespace {
    std::string g_dataRoot = "/multigauge";
    std::unique_ptr<PackageManager> g_packages;

    YGConfigRef createYogaConfig() {
        YGConfigRef config = YGConfigNew();
        YGConfigSetUseWebDefaults(config, false);
        return config;
    }

    HandlePool<RuntimeContext> contexts;
    bool initialized = false;
    std::chrono::microseconds lastElapsed{};
    YGConfigRef yogaConfig = nullptr;

    io::FileSystem* g_fs = nullptr;
    io::Time* g_time = nullptr;

}

bool init(io::FileSystem& fs, io::Time& time, const AppConfig& config, io::Logger* logger) {
    if (initialized) return true;

    g_fs = &fs;
    g_time = &time;
    g_dataRoot = config.dataRoot.empty() ? "/multigauge" : config.dataRoot;

    io::setLogger(logger);

    if (logger) {
        if (!logger->init()) return false;
    }

    if (!g_fs->init()) return false;

    g_packages = std::make_unique<PackageManager>(*g_fs, g_dataRoot);
    g_packages->rebuildLibrary();
    contexts.clear();
    yogaConfig = createYogaConfig();
    initialized = true;

    lastElapsed = g_time->elapsed();

    return true;
}

void shutdown() {
    contexts.clear();
    g_packages.reset();
    if (yogaConfig) {
        YGConfigFree(yogaConfig);
        yogaConfig = nullptr;
    }
    initialized = false;
    lastElapsed = {};
    g_time = nullptr;
    g_fs = nullptr;
}

void frame() {
    if (!initialized) return;

    const std::chrono::microseconds now = g_time->elapsed();
    const std::chrono::microseconds delta = now >= lastElapsed ? now - lastElapsed : std::chrono::microseconds{};
    lastElapsed = now;

    graphics::TimeColor::setElapsed(now);
    for (auto& context : contexts) context.frame(delta);
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
bool loadGaugeFaceFromValue(json::Reader value, std::unique_ptr<gauge::GaugeFace>& outFace) {
    if (!value.isObject()) return false;

    auto face = std::make_unique<gauge::GaugeFace>();
    if (!face->load(value)) return false;
    outFace = std::move(face);
    return true;
}
}

bool setGaugeScreen(ContextId id, const std::string& json) {
    RuntimeContext* context = getContext(id);
    if (!context) return false;

    json::Document doc = json::parse(json);
    if (!doc.valid() || !doc.root().isObject()) return false;

    std::unique_ptr<gauge::GaugeFace> face;
    if (!loadGaugeFaceFromValue(doc.root(), face)) return false;

    auto screen = std::make_unique<GaugeScreen>();
    screen->setFace(std::move(face));
    return context->setScreen(std::move(screen));
}

bool setGaugeScreen(ContextId id, const std::string& packageId, const std::string& faceId) {
    if (!g_packages) return false;

    Result faceResult = g_packages->getFace(packageId, faceId);
    if (!faceResult.ok) return false;

    std::unique_ptr<gauge::GaugeFace> face;
    if (!loadGaugeFaceFromValue(faceResult.data.root(), face)) return false;

    auto screen = std::make_unique<GaugeScreen>();
    screen->setFace(std::move(face));
    RuntimeContext* context = getContext(id);
    if (!context) return false;
    return context->setScreen(std::move(screen));
}

bool setEditorScreen(ContextId id, editor::EditorId editorId, editor::NodeId faceId) {
    RuntimeContext* context = getContext(id);
    if (!context) return false;
    if (!editor::exists(editorId) || !editor::isFace(editorId, faceId)) return false;

    auto screen = std::make_unique<EditorScreen>(editorId, faceId);
    return context->setScreen(std::move(screen));
}

bool listPackages(std::vector<PackageSummary>& out) {
    if (!g_packages) return false;
    return g_packages->listPackages(out);
}

bool listFaces(const std::string& packageId, std::vector<FaceSummary>& out) {
    if (!g_packages) return false;
    return g_packages->listFaces(packageId, out);
}

Result getPackage(const std::string& packageId) {
    if (!g_packages) return Error("App not initialized");
    return g_packages->getPackage(packageId);
}

Result importPackage(const std::string& json) {
    if (!g_packages) return Error("App not initialized");
    return g_packages->importPackage(json);
}

Result importPackage(json::Reader package) {
    if (!g_packages) return Error("App not initialized");
    return g_packages->importPackage(package);
}

Result exportPackage(const std::string& packageId) {
    if (!g_packages) return Error("App not initialized");
    return g_packages->exportPackage(packageId);
}

Result removePackage(const std::string& packageId) {
    if (!g_packages) return Error("App not initialized");
    return g_packages->removePackage(packageId);
}

} // namespace mg
