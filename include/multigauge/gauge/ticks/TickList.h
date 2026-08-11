#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <multigauge/constants.h>
#include <multigauge/gauge/ticks/RootTick.h>
#include <multigauge/gauge/ticks/SubTick.h>
#include <multigauge/graphics/Graphics.h>
#include <multigauge/utils/Math.h>
#include <optional>
#include <utility>
#include <vector>

namespace mg::gauge {
using ::mg::Line;
using ::mg::Point;
using ::mg::graphics::Graphics;
using ::mg::graphics::PaintTimeline;
using ::mg::graphics::ResolvedPaint;
using ::mg::utils::inRange;
using ::mg::utils::lerp;

/// @brief Stores and renders a hierarchical list of gauge ticks.
class TickList : public ::mg::PropertyObject {
    MG_EDITOR_NAME("Tick List")
    RootTick root;
    std::vector<SubTick> subs;
    LengthAlignment align = LengthAlignment::OUTER;
    float offset = 0;
    float displayValue = 0;
    float lengthFactor = 2;
    float thicknessFactor = 0;
    float textSizeFactor = 1;
    float leftHighlightBase = 0;
    float leftHighlightFactor = 1;
    float leftHighlightDistance = 1000;
    float rightHighlightBase = 0;
    float rightHighlightFactor = 1;
    float rightHighlightDistance = 1000;
    float getLength(uint8_t index) const;
    float getThickness(uint8_t index) const;
    const TickStyle& getStyle(uint8_t index) const;
    const PaintTimeline& getColor(uint8_t index) const;
    const std::optional<TickValueStyle>& getValueStyle(uint8_t index) const;

    std::vector<std::vector<float>> getTickPositions(float startValue, float endValue) const {
        auto all = std::vector<std::vector<float>>(subs.size() + 1);
        auto positions = root.getPositions(startValue, endValue, offset);
        const float interval = root.getInterval(startValue, endValue);
        for (float position : positions) {
            if (inRange(position, 0.f, 1.f)) all[0].push_back(position);
            getSeqTickPositions(position, position + interval, 0, all);
        }
        return all;
    }

    void getSeqTickPositions(
        float start,
        float end,
        uint8_t index,
        std::vector<std::vector<float>>& out
    ) const {
        if (index >= subs.size()) return;
        const SubTick& tick = subs[index];
        auto positions = tick.getPositions(start, end);
        const float interval = tick.getInterval(start, end);
        for (float position : positions) {
            if (inRange(position, 0.f, 1.f)) out[index + 1].push_back(position);
            getSeqTickPositions(position, position + interval, index + 1, out);
        }
    }

    void drawLineTick(
        Graphics& g,
        Line<float> line,
        float thickness,
        const ResolvedPaint& paint
    ) const {
        g.setPaint(paint);
        g.drawLine(line.toInt(), thickness);
    }

    void drawCircleTick(Graphics&, Line<float>, float, const ResolvedPaint&) const {}

    void drawTriangleTick(Graphics&, Line<float>, float, const ResolvedPaint&) const {}

    void drawCircularTick(Graphics&, uint8_t, Point<float>, float, float, float, float) const;

    float getHighlightFactor(float value) const {
        const float delta = value - displayValue;
        if (delta < 0)
            return leftHighlightDistance <= 0 || delta <= -leftHighlightDistance
                       ? leftHighlightBase
                       : lerp(leftHighlightBase,
                              leftHighlightFactor,
                              1.f -
                                  std::min(1.f,
                                           static_cast<float>(abs(delta)) / leftHighlightDistance));
        if (delta > 0)
            return rightHighlightDistance <= 0 || delta >= rightHighlightDistance
                       ? rightHighlightBase
                       : lerp(rightHighlightBase,
                              rightHighlightFactor,
                              1.f - std::min(1.f,
                                             static_cast<float>(abs(delta)) /
                                                 rightHighlightDistance));
        return std::max(leftHighlightFactor, rightHighlightFactor);
    }

public:
    /// @brief Draws ticks around a circular value range.
    void drawCircular(
        Graphics& graphics,
        Point<float> center,
        float radius,
        float startAngle,
        float endAngle,
        float startValue,
        float endValue
    ) const {
        auto positions = getTickPositions(startValue, endValue);
        const float start = startAngle * (PI / 180.f), end = endAngle * (PI / 180.f);
        for (int i = static_cast<int>(positions.size()) - 1; i >= 0; --i)
            for (float tickPosition : positions[i])
                drawCircularTick(
                    graphics,
                    static_cast<uint8_t>(i),
                    center,
                    radius,
                    tickPosition,
                    lerp(start, end, tickPosition),
                    lerp(startValue, endValue, tickPosition)
                );
    }

    /// @brief Updates the value used for tick highlighting.
    void setValueView(float value) { displayValue = value; }

    MG_PROPS_BEGIN()
        MG_PROP(root, "root", "Root Tick", "First tick drawn.")
        MG_PROP(subs, "subs", "Sub Ticks", "Sequentially drawn ticks.")
        MG_PROP(offset, "offset", "Offset", "Value offset to start ticks from.")
    MG_PROPS_END()
};
} // namespace mg::gauge
