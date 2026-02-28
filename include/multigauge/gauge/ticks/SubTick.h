#pragma once

#include <optional>

#include <multigauge/gauge/ticks/TickStyle.h>
#include <multigauge/utils.h>

struct SubTick : public Editable {
    MG_EDITOR_NAME("Tick")
    
    int divisions = 1;

    std::optional<float> length;
    std::optional<float> thickness;

    std::optional<TickStyle> style;
    std::optional<PaintTimeline> color;

    std::optional<TickValueStyle> valueStyle;

    MG_EDITABLE_BEGIN()
        MG_EDITABLE_PROP(divisions)
        MG_EDITABLE_PROP(length)
        MG_EDITABLE_PROP(thickness)
        MG_EDITABLE_PROP(color)
    MG_EDITABLE_END()

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