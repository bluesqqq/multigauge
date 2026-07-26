#include <multigauge/graphics/colors/ColorTimeline.h>

#include <algorithm>
#include <cmath>
#include <utility>

#include <multigauge/graphics/colors/StaticColor.h>

namespace mg::graphics {

ColorKeyframe::ColorKeyframe(OwnedColor color, float position) : position(position), color(std::move(color)) {}
ColorKeyframe::ColorKeyframe(const ColorKeyframe& other)
    : position(other.position), color(other.color ? other.color->clone() : nullptr) {}
ColorKeyframe& ColorKeyframe::operator=(const ColorKeyframe& other) {
    if (this != &other) {
        position = other.position;
        color = other.color ? other.color->clone() : nullptr;
    }
    return *this;
}

ColorTimeline::ColorTimeline(rgba color) { addKeyframe(color, 0.0F); }
ColorTimeline::ColorTimeline(OwnedColor color) { addKeyframe(std::move(color), 0.0F); }
ColorTimeline::ColorTimeline(const ColorTimeline& other) : keyframes(other.keyframes) {}
ColorTimeline& ColorTimeline::operator=(const ColorTimeline& other) { keyframes = other.keyframes; return *this; }

void ColorTimeline::clear() { keyframes.clear(); }

bool ColorTimeline::addKeyframe(rgba color, float position) {
    return addKeyframe(std::make_unique<StaticColor>(color), position);
}

bool ColorTimeline::addKeyframe(OwnedColor color, float position) {
    return addKeyframe(ColorKeyframe(std::move(color), position));
}

bool ColorTimeline::addKeyframe(ColorKeyframe keyframe) {
    if (!keyframe.color || !std::isfinite(keyframe.position) || keyframe.position < 0.0F || keyframe.position > 1.0F) return false;
    const auto position = std::lower_bound(keyframes.begin(), keyframes.end(), keyframe.position,
        [](const ColorKeyframe& existing, float position) { return existing.position < position; });
    if (position != keyframes.end() && position->position == keyframe.position) return false;
    keyframes.insert(position, std::move(keyframe));
    return true;
}

std::size_t ColorTimeline::size() const noexcept { return keyframes.size(); }
bool ColorTimeline::empty() const noexcept { return keyframes.empty(); }

bool ColorTimeline::valid() const noexcept {
    float previous = -1.0F;
    for (const ColorKeyframe& keyframe : keyframes) {
        if (!keyframe.color || !std::isfinite(keyframe.position) || keyframe.position < 0.0F || keyframe.position > 1.0F || keyframe.position <= previous) return false;
        previous = keyframe.position;
    }
    return true;
}

std::size_t ColorTimeline::indexAt(float position) const noexcept {
    const auto upper = std::upper_bound(keyframes.begin(), keyframes.end(), position,
        [](float position, const ColorKeyframe& keyframe) { return position < keyframe.position; });
    return upper == keyframes.begin() ? 0 : static_cast<std::size_t>((upper - keyframes.begin()) - 1);
}

rgba ColorTimeline::sample(float position, const ColorResolver::Frame& frame) const noexcept {
    if (!frame.valid() || keyframes.empty() || !valid() || !std::isfinite(position)) return rgba{0, 0, 0, 0};
    position = std::clamp(position, 0.0F, 1.0F);
    if (position <= keyframes.front().position) return frame.resolve(*keyframes.front().color);
    if (position >= keyframes.back().position) return frame.resolve(*keyframes.back().color);

    const std::size_t index = indexAt(position);
    const ColorKeyframe& left = keyframes[index];
    const ColorKeyframe& right = keyframes[index + 1];
    const float t = (position - left.position) / (right.position - left.position);
    return frame.resolve(*left.color).blended(frame.resolve(*right.color), t);
}

PaintTimeline::PaintTimeline(ColorTimeline fill, ColorTimeline stroke, float thickness)
    : fill(std::move(fill)), stroke(std::move(stroke)), thickness(thickness) {}

ResolvedPaint PaintTimeline::sample(float position, const ColorResolver::Frame& frame) const noexcept {
    return {
        .fillEnabled = !fill.empty(),
        .fill = fill.sample(position, frame),
        .strokeEnabled = !stroke.empty(),
        .stroke = stroke.sample(position, frame),
        .thickness = std::max(0.0F, thickness),
    };
}

} // namespace mg::graphics
