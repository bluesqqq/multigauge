#include <multigauge/graphics/colors/TimeColor.h>

#include <cmath>
#include <cstdint>
#include <limits>
#include <utility>

namespace mg::graphics {

TimeColor::TimeColor() = default;
TimeColor::TimeColor(ColorTimeline timeline, LoopType loopType, float periodMs)
    : timeline(std::move(timeline)), loopType(loopType), periodMs(periodMs) {}
OwnedColor TimeColor::clone() const { return std::make_unique<TimeColor>(*this); }

rgba TimeColor::resolveUncached(const ColorResolver::Frame& frame) const noexcept {
    const ColorFrame* data = frame.data();
    if (!data || !std::isfinite(periodMs) || periodMs <= 0.0F) return rgba{0, 0, 0, 0};
    constexpr double maxPeriodMs = static_cast<double>(std::numeric_limits<std::int64_t>::max() / 2) / 1000.0;
    if (static_cast<double>(periodMs) > maxPeriodMs) return rgba{0, 0, 0, 0};
    const auto period = static_cast<std::int64_t>(std::llround(static_cast<double>(periodMs) * 1000.0));
    if (period <= 0) return rgba{0, 0, 0, 0};

    const std::int64_t elapsed = data->elapsed().count() >= 0 ? data->elapsed().count() : 0;
    const std::int64_t phase = elapsed % period;
    float normalized = static_cast<float>(static_cast<double>(phase) / static_cast<double>(period));
    switch (loopType) {
        case LoopType::Forward: break;
        case LoopType::Reverse: normalized = 1.0F - normalized; break;
        case LoopType::PingPong:
            normalized = static_cast<float>(static_cast<double>(elapsed % (period * 2)) / static_cast<double>(period));
            if (normalized > 1.0F) normalized = 2.0F - normalized;
            break;
    }
    return timeline.sample(normalized, frame);
}

} // namespace mg::graphics
