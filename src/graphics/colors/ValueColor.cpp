#include <multigauge/graphics/colors/ValueColor.h>

#include <algorithm>
#include <utility>

namespace mg::graphics {

ValueColor::ValueColor(::mg::ValueRef value, ColorTimeline timeline) : timeline(std::move(timeline)), value(std::move(value)) {}
OwnedColor ValueColor::clone() const { return std::make_unique<ValueColor>(*this); }

rgba ValueColor::resolveUncached(const ColorResolver::Frame& frame) const noexcept {
    const ColorFrame* data = frame.data();
    if (!data || !value) return rgba{0, 0, 0, 0};
    const float minimum = value.minimum();
    const float maximum = value.maximum();
    const float span = maximum - minimum;
    if (span <= 0.0F) return rgba{0, 0, 0, 0};
    const float normalized = std::clamp((data->values().value(value.handle()) - minimum) / span, 0.0F, 1.0F);
    return timeline.sample(normalized, frame);
}

} // namespace mg::graphics
