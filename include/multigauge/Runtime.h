#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <multigauge/container/GenerationalHandle.h>
#include <multigauge/editor/Api.h>
#include <multigauge/graphics/UserPalette.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/io/Logger.h>
#include <multigauge/io/Time.h>
#include <multigauge/json/Json.h>
#include <multigauge/runtime/PackageManager.h>

namespace mg {

class Screen;
namespace graphics { class GraphicsContext; }

using ContextId = GenerationalHandle<struct ContextTag>;

/// @brief Configures a Runtime instance.
struct RuntimeConfig {
    std::string dataRoot = "/multigauge";
};

/// @brief Owns the long-lived state of one Multigauge runtime instance.
class Runtime {
public:
    class State;

    Runtime(io::FileSystem& fs, io::Time& time, RuntimeConfig config = {}, io::Logger* logger = nullptr);
    ~Runtime();

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    [[nodiscard]] bool init();
    void shutdown();
    [[nodiscard]] bool initialized() const noexcept;
    void frame();

    [[nodiscard]] bool setUserColor(std::size_t slot, graphics::rgba color);
    [[nodiscard]] graphics::rgba userColor(std::size_t slot) const;

    [[nodiscard]] ContextId addContext(graphics::GraphicsContext& graphics);
    [[nodiscard]] bool removeContext(ContextId id);
    [[nodiscard]] bool hasContext(ContextId id) const;
    [[nodiscard]] std::size_t contextCount() const noexcept;

    [[nodiscard]] bool setScreen(ContextId id, std::unique_ptr<Screen> screen);
    [[nodiscard]] bool clearScreen(ContextId id);
    [[nodiscard]] bool hasScreen(ContextId id) const;
    [[nodiscard]] bool setGaugeScreen(ContextId id, const std::string& json);
    [[nodiscard]] bool setGaugeScreen(ContextId id, const std::string& packageId, const std::string& faceId);
    [[nodiscard]] bool setEditorScreen(ContextId id, editor::EditorId editorId, editor::NodeId faceId);

    [[nodiscard]] bool listPackages(std::vector<PackageSummary>& out) const;
    [[nodiscard]] bool listFaces(const std::string& packageId, std::vector<FaceSummary>& out) const;
    [[nodiscard]] Result getPackage(const std::string& packageId) const;
    [[nodiscard]] Result importPackage(const std::string& json);
    [[nodiscard]] Result importPackage(json::Reader package);
    [[nodiscard]] Result exportPackage(const std::string& packageId) const;
    [[nodiscard]] Result removePackage(const std::string& packageId);

private:
    std::unique_ptr<State> state_;
};

} // namespace mg
