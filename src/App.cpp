#include <multigauge/App.h>

#include "AppPaths.h"

#include <multigauge/runtime/PackageManager.h>

#include <multigauge/container/HandlePool.h>
#include <multigauge/editor/Api.h>
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/graphics/UserPalette.h>
#include <multigauge/io/Log.h>
#include <multigauge/runtime/RuntimeContext.h>
#include <multigauge/screens/EditorScreen.h>
#include <multigauge/screens/GaugeScreen.h>
#include <multigauge/utils/Json.h>

#include <chrono>
#include <utility>

namespace mg {

namespace {
std::string g_dataRoot = "/multigauge";
std::unique_ptr<PackageManager> g_packages;

HandlePool<RuntimeContext, ContextId> contexts;
bool initialized = false;
std::chrono::microseconds lastElapsed{};
graphics::UserPalette userPalette;

io::FileSystem* g_fs = nullptr;
io::Time* g_time = nullptr;

} // namespace

bool init(io::FileSystem& fs, io::Time& time, const AppConfig& config, io::Logger* logger) {
    constexpr const char* TAG = "init";

    io::setLogger(logger);

    if (initialized) {
        LOG_WARN(TAG, "Runtime already initialized.");
        return true;
    }

    if (logger && !logger->init()) return false;

    LOG_INFO(TAG, "Logger successfully initialized.");

    g_fs = &fs;
    g_time = &time;
    g_dataRoot = config.dataRoot.empty() ? "/multigauge" : config.dataRoot;

    LOG_INFO(TAG, "Initializing runtime: dataRoot=`{}`.", g_dataRoot.c_str());

    LOG_INFO(TAG, "Initializing filesystem...");

    if (!g_fs->init()) {
        LOG_ERROR(TAG, "Failed to initialize filesystem.");
        return false;
    }

    LOG_INFO(TAG, "Filesystem successfully initialized.");

    g_packages = std::make_unique<PackageManager>(*g_fs, g_dataRoot);
    g_packages->rebuildLibrary();

    contexts.clear();

    initialized = true;
    lastElapsed = g_time->elapsed();

    LOG_INFO(TAG, "Runtime successfully initialized.");

    return true;
}

void shutdown() {
    constexpr const char* TAG = "shutdown";

    LOG_INFO(TAG, "Shutting the runtime down...");

    contexts.clear();
    g_packages.reset();
    initialized = false;
    lastElapsed = {};
    g_time = nullptr;
    g_fs = nullptr;
}

void frame() {
    if (!initialized) return;

    const std::chrono::microseconds now = g_time->elapsed();
    const std::chrono::microseconds delta =
        now >= lastElapsed ? now - lastElapsed : std::chrono::microseconds{};
    lastElapsed = now;

    for (auto& context : contexts)
        context.frame(delta, now);
}

bool setUserColor(std::size_t slot, graphics::rgba color) {
    return userPalette.setColor(slot, color);
}

graphics::rgba getUserColor(std::size_t slot) {
    return userPalette.color(slot);
}

ContextId addContext(graphics::GraphicsContext& graphics) {
    constexpr const char* TAG = "addContext";

    if (!initialized) {
        LOG_ERROR(TAG, "Cannot add context: runtime is not initialized.");
        return {};
    }

    return contexts.emplace(graphics, *g_fs, userPalette);
}

bool removeContext(ContextId id) {
    constexpr const char* TAG = "removeContext";

    if (!contexts.remove(id)) {
        LOG_WARN(TAG, "Cannot remove invalid graphics context.");
        return false;
    }

    LOG_DEBUG(TAG, "Graphics context removed.");
    return true;
}

RuntimeContext* getContext(ContextId id) {
    return contexts.get(id);
}

bool hasContext(ContextId id) {
    return contexts.exists(id);
}

std::size_t contextCount() {
    return contexts.size();
}

bool setScreen(ContextId id, std::unique_ptr<Screen> screen) {
    constexpr const char* TAG = "setScreen";

    if (!screen) {
        LOG_WARN(TAG, "Cannot set screen: screen is null.");
        return false;
    }

    RuntimeContext* context = getContext(id);
    if (!context) {
        LOG_WARN(TAG, "Cannot set screen: invalid context.");
        return false;
    }

    if (!context->setScreen(std::move(screen))) {
        LOG_ERROR(TAG, "Failed to install screen into context.");
        return false;
    }

    LOG_DEBUG(TAG, "Screen set.");
    return true;
}

bool clearScreen(ContextId id) {
    constexpr const char* TAG = "clearScreen";

    RuntimeContext* context = getContext(id);
    if (!context) {
        LOG_WARN(TAG, "Cannot clear screen: invalid context.");
        return false;
    }

    context->clearScreen();

    LOG_DEBUG(TAG, "Screen cleared.");
    return true;
}

bool hasScreen(ContextId id) {
    constexpr const char* TAG = "hasScreen";

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
} // namespace

bool setGaugeScreen(ContextId id, const std::string& json) {
    constexpr const char* TAG = "setGaugeScreen";

    RuntimeContext* context = getContext(id);
    if (!context) {
        LOG_WARN(TAG, "Cannot set gauge screen: invalid context.");
        return false;
    }

    json::Document doc = json::parse(json);
    if (!doc.valid() || !doc.root().isObject()) {
        LOG_ERROR(TAG, "Cannot set gauge screen: invalid gauge JSON.");
        return false;
    }

    std::unique_ptr<gauge::GaugeFace> face;
    if (!loadGaugeFaceFromValue(doc.root(), face)) {
        LOG_ERROR(TAG, "Cannot set gauge screen: failed to load gauge face.");
        return false;
    }

    auto screen = std::make_unique<GaugeScreen>();
    screen->setFace(std::move(face));

    if (!context->setScreen(std::move(screen))) {
        LOG_ERROR(TAG, "Failed to install gauge screen into context.");
        return false;
    }

    LOG_DEBUG(TAG, "Gauge screen set from JSON.");
    return true;
}

bool setGaugeScreen(ContextId id, const std::string& packageId, const std::string& faceId) {
    constexpr const char* TAG = "setGaugeScreen";

    if (!g_packages) {
        LOG_ERROR(TAG, "Cannot set gauge screen: runtime is not initialized.");
        return false;
    }

    RuntimeContext* context = getContext(id);
    if (!context) {
        LOG_WARN(TAG, "Cannot set gauge screen: invalid context.");
        return false;
    }

    Result faceResult = g_packages->getFace(packageId, faceId);
    if (!faceResult.ok) {
        LOG_ERROR(
            TAG,
            "Failed to load gauge face: package=`{}`, face=`{}`.",
            packageId.c_str(),
            faceId.c_str()
        );
        return false;
    }

    std::unique_ptr<gauge::GaugeFace> face;
    if (!loadGaugeFaceFromValue(faceResult.data.root(), face)) {
        LOG_ERROR(TAG, "Cannot set gauge screen: failed to load gauge face.");
        return false;
    }

    auto screen = std::make_unique<GaugeScreen>();
    screen->setFace(std::move(face));

    if (!context->setScreen(std::move(screen))) {
        LOG_ERROR(TAG, "Failed to install gauge screen into context.");
        return false;
    }

    LOG_DEBUG(TAG, "Gauge screen set from package.");
    return true;
}

bool setEditorScreen(ContextId id, editor::EditorId editorId, editor::NodeId faceId) {
    constexpr const char* TAG = "setEditorScreen";

    RuntimeContext* context = getContext(id);
    if (!context) {
        LOG_WARN(TAG, "Cannot set editor screen: invalid context.");
        return false;
    }

    if (!editor::exists(editorId)) {
        LOG_WARN(TAG, "Cannot set editor screen: invalid editor.");
        return false;
    }

    if (!editor::isFace(editorId, faceId)) {
        LOG_WARN(TAG, "Cannot set editor screen: node is not a valid face.");
        return false;
    }

    auto screen = std::make_unique<EditorScreen>(editorId, faceId);

    if (!context->setScreen(std::move(screen))) {
        LOG_ERROR(TAG, "Failed to install editor screen into context.");
        return false;
    }

    LOG_DEBUG(TAG, "Editor screen set.");
    return true;
}

bool listPackages(std::vector<PackageSummary>& out) {
    constexpr const char* TAG = "listPackages";
    
    if (!g_packages) {
        LOG_ERROR(TAG, "Cannot list packages: runtime is not initialized.");
        return false;
    }

    return g_packages->listPackages(out);
}

bool listFaces(const std::string& packageId, std::vector<FaceSummary>& out) {
    constexpr const char* TAG = "listFaces";
    
    if (!g_packages) {
        LOG_ERROR(TAG, "Cannot list faces: runtime is not initialized.");
        return false;
    }

    return g_packages->listFaces(packageId, out);
}

Result getPackage(const std::string& packageId) {
    constexpr const char* TAG = "getPackage";

    if (!g_packages) {
        LOG_ERROR(TAG, "Cannot get package: runtime is not initialized.");
        return Error("App not initialized");
    }

    return g_packages->getPackage(packageId);
}

Result importPackage(const std::string& json) {
    constexpr const char* TAG = "importPackage";

    if (!g_packages) {
        LOG_ERROR(TAG, "Cannot import package: runtime is not initialized.");
        return Error("App not initialized");
    }

    return g_packages->importPackage(json);
}

Result importPackage(json::Reader package) {
    constexpr const char* TAG = "importPackage";

    if (!g_packages) {
        LOG_ERROR(TAG, "Cannot import package: runtime is not initialized.");
        return Error("App not initialized");
    }

    return g_packages->importPackage(package);
}

Result exportPackage(const std::string& packageId) {
    constexpr const char* TAG = "exportPackage";

    if (!g_packages) {
        LOG_ERROR(TAG, "Cannot export package: runtime is not initialized.");
        return Error("App not initialized");
    }

    return g_packages->exportPackage(packageId);
}

Result removePackage(const std::string& packageId) {
    constexpr const char* TAG = "removePackage";

    if (!g_packages) {
        LOG_ERROR(TAG, "Cannot remove package: runtime is not initialized.");
        return Error("App not initialized");
    }
    
    return g_packages->removePackage(packageId);
}

} // namespace mg
