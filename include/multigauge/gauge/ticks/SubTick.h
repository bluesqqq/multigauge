#pragma once

#include <optional>

#include <multigauge/gauge/ticks/TickStyle.h>
#include <multigauge/utils.h>

namespace mg::gauge {

struct SubTick : public ::PropertyObject {
    MG_EDITOR_NAME("Tick")
    
    int divisions = 1;

    std::optional<float> length;
    std::optional<float> thickness;

    std::optional<TickStyle> style;
    std::optional<PaintTimeline> paint;

    std::optional<TickValueStyle> valueStyle;

    MG_PROPS_BEGIN()
    MG_PROP(divisions, "divisions", "Divisions", "Number of times to divide ticks. Make null to inherit from parent.")
    MG_PROP(length, "length", "Length", "Length in pixels. Make null to inherit from parent.")
    MG_PROP(thickness, "thickness", "Thickness", "Thickness in pixels. Make null to inherit from parent.")
    MG_PROP(paint, "paint", "Paint", "Paint options for this tick. Make null to inherit from parent.")
    MG_PROPS_END()

    SubTick() = default;

    float getInterval(float lower, float upper) const { return (upper - lower) / (divisions + 1); }

    std::vector<float> getPositions(float lower, float upper) const {
        std::vector<float> positions = {};

        if (divisions <= 0) return positions;

        const float interval = getInterval(lower, upper);

        for (int i = 0; i <= divisions; i++) {
            const float position = lower + (i * interval);

            positions.push_back(position);
        }

        return positions;
    }
};

} // namespace mg::gauge

using SubTick = mg::gauge::SubTick;
