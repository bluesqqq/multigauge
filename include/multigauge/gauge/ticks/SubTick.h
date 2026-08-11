#pragma once

#include <multigauge/gauge/ticks/TickStyle.h>
#include <optional>
#include <vector>

namespace mg::gauge {
using ::mg::graphics::PaintTimeline;

/// @brief Defines an inherited child tick style.
struct SubTick : public ::mg::PropertyObject {
    MG_EDITOR_NAME("Tick")
    int divisions = 1;
    std::optional<float> length;
    std::optional<float> thickness;
    std::optional<TickStyle> style;
    std::optional<PaintTimeline> paint;
    std::optional<TickValueStyle> valueStyle;
    /// @brief Returns the interval between child ticks.
    float getInterval(float lower, float upper) const { return (upper - lower) / (divisions + 1); }

    /// @brief Returns child tick positions for an interval.
    std::vector<float> getPositions(float lower, float upper) const {
        std::vector<float> positions;
        if (divisions <= 0) return positions;
        const float interval = getInterval(lower, upper);
        for (int i = 0; i <= divisions; ++i)
            positions.push_back(lower + (i * interval));
        return positions;
    }

    MG_PROPS_BEGIN()
    MG_PROP(divisions,
            "divisions",
            "Divisions",
            "Number of times to divide ticks. Make null to inherit from parent.")
    MG_PROP(length, "length", "Length", "Length in pixels. Make null to inherit from parent.")
    MG_PROP(thickness,
            "thickness",
            "Thickness",
            "Thickness in pixels. Make null to inherit from parent.")
    MG_PROP(paint,
            "paint",
            "Paint",
            "Paint options for this tick. Make null to inherit from parent.")
    MG_PROPS_END()
};
} // namespace mg::gauge
