#pragma once

#include <multigauge/graphics/colors/ColorTimeline.h>

namespace mg::graphics {

class TimeColor final : public Color {
    MG_EDITOR_NAME("Time Color")
    MG_TYPE_ID("time")

public:
    enum class LoopType { Forward, Reverse, PingPong };

    TimeColor();
    TimeColor(ColorTimeline timeline, LoopType loopType = LoopType::Forward, float periodMs = 1000.0F);
    TimeColor(const TimeColor&) = default;
    TimeColor& operator=(const TimeColor&) = default;
    OwnedColor clone() const override;

protected:
    rgba resolveUncached(const ColorResolver::Frame& frame) const noexcept override;

private:
    ColorTimeline timeline;
    LoopType loopType = LoopType::Forward;
    float periodMs = 1000.0F;

    MG_PROPS_PARENT(Color)
    MG_PROPS_BEGIN()
    MG_PROP(timeline, "timeline", "Gradient", "Normalized animated gradient.")
    MG_PROP(loopType, "loop", "Loop", "Type of looping to use.")
    MG_PROP(periodMs, "periodMs", "Period", "Animation period in milliseconds.")
    MG_PROPS_END()
};

} // namespace mg::graphics

namespace mg {

template<> struct EnumTraits<graphics::TimeColor::LoopType> {
    static constexpr EnumOption<graphics::TimeColor::LoopType> options[] = {
        { graphics::TimeColor::LoopType::Forward, "forward", "Forward" },
        { graphics::TimeColor::LoopType::Reverse, "reverse", "Reverse" },
        { graphics::TimeColor::LoopType::PingPong, "pingpong", "Ping Pong" },
    };
};

CODEC_BEGIN(graphics::TimeColor::LoopType)
    DECODE() { return decodeEnum(v, out); }
    ENCODE() { return encodeEnum(out, v); }
CODEC_END()

} // namespace mg
