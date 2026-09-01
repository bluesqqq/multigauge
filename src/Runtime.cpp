#include <multigauge/Runtime.h>

#include <multigauge/graphics/UserPalette.h>
#include <multigauge/io/Log.h>

#include <utility>

namespace mg {

Runtime::Runtime(
    io::FileSystem& fs,
    io::Time& time,
    RuntimeConfig config,
    io::Logger* logger
) : dataRoot_(config.dataRoot.empty() ? "/multigauge" : std::move(config.dataRoot)),
    sensors_(fs, dataRoot_),
    fs_(fs),
    time_(time),
    logger_(logger) {}

Runtime::~Runtime() { shutdown(); }

bool Runtime::init() {
    constexpr const char* TAG = "init";
    if (initialized_) return true;
    
    io::setLogger(logger_);
    if (logger_ && !logger_->init()) return false;
    
    if (!fs_.init()) {
        LOG_ERROR(TAG, "Failed to initialize filesystem.");
        return false;
    }

    packages_ = std::make_unique<package::Manager>(fs_, dataRoot_);
    packages_->rebuildLibrary();
    
    if (!sensors_.load()) return false;

    contexts_ = std::make_unique<context::Manager>(
        fs_, dataRoot_, userPalette_, *packages_
#if MG_BUILD_EDITOR
        , editors_
#endif
    );
    lastElapsed_ = time_.elapsed();
    initialized_ = true;
    return true;
}

void Runtime::shutdown() {
    if (!initialized_) return;
    contexts_.reset();
    packages_.reset();
    initialized_ = false;
    lastElapsed_ = {};
}

bool Runtime::initialized() const noexcept { return initialized_; }

void Runtime::frame() {
    if (!initialized_) return;

    const auto now = time_.elapsed();
    const auto delta = now >= lastElapsed_ ? now - lastElapsed_ : std::chrono::microseconds{};
    lastElapsed_ = now;
    sensors_.update(delta, now);
    contexts_->frame(delta, now);
}

package::Manager& Runtime::packages() { return *packages_; }

const package::Manager& Runtime::packages() const { return *packages_; }

context::Manager& Runtime::contexts() { return *contexts_; }

const context::Manager& Runtime::contexts() const { return *contexts_; }

sensor::Manager& Runtime::sensors() { return sensors_; }

const sensor::Manager& Runtime::sensors() const { return sensors_; }

#if MG_BUILD_EDITOR
editor::Manager& Runtime::editors() { return editors_; }

const editor::Manager& Runtime::editors() const { return editors_; }
#endif

bool Runtime::setUserColor(std::size_t slot, graphics::rgba color) {
    return userPalette_.setColor(slot, color);
}

graphics::rgba Runtime::userColor(std::size_t slot) const {
    return userPalette_.color(slot);
}

} // namespace mg
