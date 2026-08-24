#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <multigauge/context/Manager.h>
#include <multigauge/Config.h>
#if MG_BUILD_EDITOR
#include <multigauge/editor/Manager.h>
#endif
#include <multigauge/graphics/UserPalette.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/io/Logger.h>
#include <multigauge/io/Time.h>
#include <multigauge/json/Json.h>
#include <multigauge/package/Manager.h>
#include <multigauge/sensor/Manager.h>

namespace mg {

class Screen;
namespace graphics { class GraphicsContext; }

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

    [[nodiscard]] package::Manager& packages();
    [[nodiscard]] const package::Manager& packages() const;

    [[nodiscard]] context::Manager& contexts();
    [[nodiscard]] const context::Manager& contexts() const;

    [[nodiscard]] sensor::Manager& sensors();
    [[nodiscard]] const sensor::Manager& sensors() const;

#if MG_BUILD_EDITOR
    [[nodiscard]] editor::Manager& editors();
    [[nodiscard]] const editor::Manager& editors() const;
#endif

    [[nodiscard]] bool setUserColor(std::size_t slot, graphics::rgba color);
    [[nodiscard]] graphics::rgba userColor(std::size_t slot) const;

private:
    std::unique_ptr<State> state_;
};

} // namespace mg
