#pragma once

#include <chrono>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include <multigauge/context/Manager.h>
#include <multigauge/Config.h>
#include <multigauge/control/Manager.h>
#if MG_BUILD_EDITOR
#include <multigauge/editor/Manager.h>
#endif
#include <multigauge/graphics/UserPalette.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/io/Logger.h>
#include <multigauge/io/Time.h>
#include <multigauge/json/Json.h>
#include <multigauge/navigation/Manager.h>
#include <multigauge/package/Manager.h>
#include <multigauge/sensor/Manager.h>
#include <multigauge/settings/Manager.h>

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
    //----------[ CTOR + DTOR ]----------//

    Runtime(
        io::FileSystem& fs,
        io::Time& time,
        RuntimeConfig config = {},
        io::Logger* logger = nullptr
    );

    ~Runtime();

    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;

    //----------[ LIFECYCLE ]----------//

    [[nodiscard]] bool init();
    void shutdown();
    [[nodiscard]] bool initialized() const noexcept;
    void frame();

    //----------[ MANAGERS ]----------//

    [[nodiscard]] package::Manager& packages();
    [[nodiscard]] const package::Manager& packages() const;

    [[nodiscard]] context::Manager& contexts();
    [[nodiscard]] const context::Manager& contexts() const;

    [[nodiscard]] navigation::Manager& navigation();
    [[nodiscard]] const navigation::Manager& navigation() const;

    [[nodiscard]] sensor::Manager& sensors();
    [[nodiscard]] const sensor::Manager& sensors() const;

    [[nodiscard]] settings::Manager& settings();
    [[nodiscard]] const settings::Manager& settings() const;

    [[nodiscard]] control::Manager& controls();
    [[nodiscard]] const control::Manager& controls() const;

#if MG_BUILD_EDITOR
    [[nodiscard]] editor::Manager& editors();
    [[nodiscard]] const editor::Manager& editors() const;
#endif

    //----------[ USER COLORS ]----------//

    [[nodiscard]] bool setUserColor(std::size_t slot, graphics::rgba color);
    [[nodiscard]] graphics::rgba userColor(std::size_t slot) const;

private:
    std::string dataRoot_;

    std::unique_ptr<package::Manager> packages_;
    sensor::Manager sensors_;
    settings::Manager settings_;
#if MG_BUILD_EDITOR
    editor::Manager editors_;
#endif
    std::unique_ptr<context::Manager> contexts_;
    std::unique_ptr<navigation::Manager> navigation_;
    std::unique_ptr<control::Manager> controls_;

    graphics::UserPalette userPalette_;

    bool initialized_ = false;
    std::chrono::microseconds lastElapsed_{};

    io::FileSystem& fs_;
    io::Time& time_;
    io::Logger* logger_;
};

} // namespace mg
