#include <multigauge/Runtime.h>

#include <multigauge/graphics/UserPalette.h>
#include <multigauge/io/Log.h>

#include <utility>

namespace mg {

class Runtime::State {
public:
    State(io::FileSystem& fs, io::Time& time, RuntimeConfig config, io::Logger* logger)
        : dataRoot(config.dataRoot.empty() ? "/multigauge" : std::move(config.dataRoot)),
          sensors(fs, dataRoot), fs(&fs), time(&time), logger(logger) {}

    std::string dataRoot;
    std::unique_ptr<package::Manager> packages;
    sensor::Manager sensors;
    editor::Manager editors;
    graphics::UserPalette userPalette;
    std::unique_ptr<context::Manager> contexts;
    bool initialized = false;
    std::chrono::microseconds lastElapsed{};
    io::FileSystem* fs;
    io::Time* time;
    io::Logger* logger;
};

Runtime::Runtime(io::FileSystem& fs, io::Time& time, RuntimeConfig config, io::Logger* logger)
    : state_(std::make_unique<State>(fs, time, std::move(config), logger)) {}

Runtime::~Runtime() { shutdown(); }

bool Runtime::init() {
    constexpr const char* TAG = "init";
    State& state = *state_;

    if (state.initialized) return true;
    io::setLogger(state.logger);
    if (state.logger && !state.logger->init()) return false;
    if (!state.fs->init()) {
        LOG_ERROR(TAG, "Failed to initialize filesystem.");
        return false;
    }

    state.packages = std::make_unique<package::Manager>(*state.fs, state.dataRoot);
    state.packages->rebuildLibrary();
    if (!state.sensors.load()) return false;
    state.contexts = std::make_unique<context::Manager>(
        *state.fs, state.dataRoot, state.userPalette, *state.packages
    );
    state.lastElapsed = state.time->elapsed();
    state.initialized = true;
    return true;
}

void Runtime::shutdown() {
    State& state = *state_;
    if (!state.initialized) return;
    state.contexts.reset();
    state.packages.reset();
    state.initialized = false;
    state.lastElapsed = {};
}

bool Runtime::initialized() const noexcept { return state_->initialized; }

void Runtime::frame() {
    State& state = *state_;
    if (!state.initialized) return;
    const auto now = state.time->elapsed();
    const auto delta = now >= state.lastElapsed ? now - state.lastElapsed : std::chrono::microseconds{};
    state.lastElapsed = now;
    state.sensors.update(delta, now);
    state.contexts->frame(delta, now);
}

package::Manager& Runtime::packages() { return *state_->packages; }
const package::Manager& Runtime::packages() const { return *state_->packages; }
context::Manager& Runtime::contexts() { return *state_->contexts; }
const context::Manager& Runtime::contexts() const { return *state_->contexts; }
sensor::Manager& Runtime::sensors() { return state_->sensors; }
const sensor::Manager& Runtime::sensors() const { return state_->sensors; }
editor::Manager& Runtime::editors() { return state_->editors; }
const editor::Manager& Runtime::editors() const { return state_->editors; }

bool Runtime::setUserColor(std::size_t slot, graphics::rgba color) {
    return state_->userPalette.setColor(slot, color);
}

graphics::rgba Runtime::userColor(std::size_t slot) const {
    return state_->userPalette.color(slot);
}

} // namespace mg
