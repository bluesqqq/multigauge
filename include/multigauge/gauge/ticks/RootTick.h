#pragma once

#include <cmath>
#include <multigauge/gauge/ticks/TickStyle.h>
#include <multigauge/properties/meta/Inspector.h>
#include <multigauge/utils/Math.h>
#include <optional>
#include <vector>

namespace mg::gauge {
using ::mg::graphics::PaintTimeline;
using ::mg::utils::ceilDivisible;
using ::mg::utils::floorDivisible;
using ::mg::utils::mapf;

/// @brief Defines the root tick style and spacing for a tick list.
struct RootTick : public ::mg::PropertyObject {
    MG_EDITOR_NAME("Tick")
    int divisions = 2;
    float interval = 1.0f;
    bool useDivisions = true;
    float length = 0;
    float thickness = 0;
    TickStyle style = TickStyle::LINE;
    PaintTimeline paint;
    std::optional<TickValueStyle> valueStyle;
    /// @brief Returns the normalized interval between root ticks.
    float getInterval(float lower, float upper) const {
        return useDivisions ? 1.0f / static_cast<float>(divisions) : interval / (upper - lower);
    }

    /// @brief Returns normalized root tick positions for a value range.
    std::vector<float> getPositions(float lower, float upper, float offset) const {
        std::vector<float> positions;
        if (!(upper > lower)) return positions;
        if (useDivisions) {
            if (divisions <= 0) return positions;
            const float step = 1.f / static_cast<float>(divisions);
            const float offsetPosition = fmod(mapf(offset, lower, upper, 0.f, 1.f), step);
            for (int i = 0; i < divisions + 1; ++i)
                positions.push_back((i * step) + offsetPosition - step);
        } else {
            if (interval <= 0) return positions;
            const float last = ceilDivisible(upper, interval, offset);
            for (float current = floorDivisible(lower, interval, offset); current <= last;
                 current += interval)
                positions.push_back(mapf(current, lower, upper, 0.f, 1.f));
        }
        return positions;
    }

    MG_PROPS_BEGIN()
    MG_PROP(divisions, "divisions")
    MG_PROP(interval, "interval")
    MG_PROP(useDivisions, "useDivisions")
    MG_PROP(length, "length")
    MG_PROP(thickness, "thickness")
    MG_PROP(paint, "paint")
    MG_PROPS_END()

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Spacing", {
        MG_PROPERTY("useDivisions", "Use Divisions", widget::boolean);
        MG_PROPERTY_IF("divisions", "Divisions", widget::number, MG_IN("useDivisions", "true"));
        MG_PROPERTY_IF("interval", "Interval", widget::number, MG_IN("useDivisions", "false"));
    });
    MG_SECTION("Appearance", {
        MG_PROPERTY("length", "Length", widget::number);
        MG_PROPERTY("thickness", "Thickness", widget::number);
        MG_PROPERTY("paint.fill", "Fill Gradient", widget::gradient);
        MG_PROPERTY("paint.stroke", "Stroke Gradient", widget::gradient);
        MG_PROPERTY("paint.thickness", "Stroke Width", widget::number);
    });
    MG_INSPECTOR_END()
#endif

public:
};
} // namespace mg::gauge
