#pragma once

#include <cstddef>
#include <vector>

#include <multigauge/graphics/colors/Color.h>

namespace mg::graphics {

struct ColorKeyframe : public ::mg::PropertyObject {
    MG_EDITOR_NAME("Gradient Stop")

    float position = 0.0F; // normalized [0, 1]
    OwnedColor color;

    MG_PROPS_BEGIN()
    MG_PROP(position, "pos", "Position", "Normalized gradient stop position.")
    MG_PROP(color, "color", "Color", "Color at this stop.")
    MG_PROPS_END()

    ColorKeyframe() = default;
    ColorKeyframe(OwnedColor color, float position);
    ColorKeyframe(const ColorKeyframe& other);
    ColorKeyframe& operator=(const ColorKeyframe& other);
    ColorKeyframe(ColorKeyframe&&) noexcept = default;
    ColorKeyframe& operator=(ColorKeyframe&&) noexcept = default;
};

/// A normalized color ramp. Position mapping belongs to ValueColor/TimeColor;
/// interpolation always happens in this fixed [0, 1] domain.
class ColorTimeline : public ::mg::PropertyObject {
    MG_EDITOR_NAME("Gradient")
    CODEC_FRIEND(ColorTimeline)

    std::vector<ColorKeyframe> keyframes;
    MG_PROPS_BEGIN()
    MG_PROP(keyframes, "keyframes", "Stops", "Normalized gradient stops.")
    MG_PROPS_END()

public:
    ColorTimeline() = default;
    explicit ColorTimeline(rgba color);
    explicit ColorTimeline(OwnedColor color);
    ColorTimeline(const ColorTimeline& other);
    ColorTimeline& operator=(const ColorTimeline& other);
    ColorTimeline(ColorTimeline&&) noexcept = default;
    ColorTimeline& operator=(ColorTimeline&&) noexcept = default;

    void clear();
    bool addKeyframe(rgba color, float position);
    bool addKeyframe(OwnedColor color, float position);
    bool addKeyframe(ColorKeyframe keyframe);

    [[nodiscard]] std::size_t size() const noexcept;
    [[nodiscard]] bool empty() const noexcept;
    [[nodiscard]] bool valid() const noexcept;
    [[nodiscard]] rgba sample(float normalizedPosition, const ColorResolver::Frame& frame) const noexcept;

private:
    [[nodiscard]] std::size_t indexAt(float normalizedPosition) const noexcept;
};

using ColorRamp = ColorTimeline;

struct ResolvedPaint {
    bool fillEnabled = false;
    rgba fill{};
    bool strokeEnabled = false;
    rgba stroke{};
    float thickness = 1.0F;
};

struct PaintTimeline : public ::mg::PropertyObject {
    ColorTimeline fill;
    ColorTimeline stroke;
    float thickness = 1.0F;

    MG_PROPS_BEGIN()
    MG_PROP(fill, "fill", "Fill", "Fill gradient.")
    MG_PROP(stroke, "stroke", "Stroke", "Stroke gradient.")
    MG_PROP(thickness, "thickness", "Thickness", "Stroke thickness.")
    MG_PROPS_END()

    PaintTimeline() = default;
    PaintTimeline(ColorTimeline fill, ColorTimeline stroke, float thickness);
    [[nodiscard]] ResolvedPaint sample(float normalizedPosition, const ColorResolver::Frame& frame) const noexcept;
};

} // namespace mg::graphics

namespace mg {

CODEC_BEGIN(graphics::ColorTimeline)
    DECODE() {
        if (!v.isObject() || v.member(TYPE_KEY).valid()) {
            graphics::OwnedColor legacy;
            if (decodeAny(v, legacy)) {
                out = graphics::ColorTimeline(std::move(legacy));
                return true;
            }
        }

        std::vector<graphics::ColorKeyframe> stops;
        if (!v.isObject() || !decodeAny(v.member("keyframes"), stops)) return false;
        graphics::ColorTimeline decoded;
        for (auto& stop : stops) {
            if (!decoded.addKeyframe(std::move(stop))) return false;
        }
        out = std::move(decoded);
        return true;
    }

    ENCODE() {
        return out.writeObject([&](json::ObjectWriter& object) {
            return object.writeValue("keyframes", [&](json::Writer& writer) { return encodeAny(writer, v.keyframes); });
        });
    }
CODEC_END()

template <> struct MgPropWidgetTraits<graphics::ColorTimeline> { static constexpr const char* value = "gradient"; };

} // namespace mg
