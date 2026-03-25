#pragma once

#include <multigauge/gauge/ticks/TickStyle.h>
#include <multigauge/utils.h>

#include <cmath>
#include <vector>
#include <optional>

struct RootTick : public PropertyObject {
    MG_EDITOR_NAME("Tick")
    
    int divisions = 2;
    float interval = 1.0f;
    bool useDivisions = true;

    float length = 0;
    float thickness= 0;

    TickStyle style = TickStyle::LINE;
    PaintTimeline paint;

    std::optional<TickValueStyle> valueStyle;

    MG_PROPS_BEGIN()
        MG_PROP(divisions, "divisions", "Divisions", "Number of times to divide ticks.", "Ticks", "Spacing")
        MG_PROP(interval, "interval", "Interval", "Gap between ticks as a value.", "Ticks", "Spacing")
        MG_PROP(useDivisions, "useDivisions", "Use Divisions", "Whether to use division or interval spacing.", "Ticks", "Spacing")
        MG_PROP(length, "length", "Length", "Length in pixels.", "Ticks", "Geometry")
        MG_PROP(thickness, "thickness", "Thickness", "Thickness in pixels.", "Ticks", "Geometry")
        MG_PROP(paint, "paint", "Paint", "Paint options for this tick.", "Appearance", "Tick Paint")
        // TODO: need to do valueStyle
    MG_PROPS_END()

    RootTick() = default;

    float getInterval(float lower, float upper) const {
        return useDivisions ? 1.0f / (float)divisions : interval / (upper - lower);
    }

    std::vector<float> getPositions(float lower, float upper, float offset) const {
        std::vector<float> positions = {};

        if (!(upper > lower)) return positions;

        if (useDivisions) {
            if (divisions <= 0) return positions;

            const float interval = 1.f / (float)divisions;

            const float offsetPosition = fmod(mapf(offset, lower, upper, 0.f, 1.f), interval);

            for (int i = 0; i < divisions + 1; i++) {
                const float position = (i * interval) + offsetPosition - interval;

                positions.push_back(position);
            }
        } else {
            if (interval <= 0) return positions;
            const float lastTickValue = ceilDivisible(upper, interval, offset);

            float currentValue = floorDivisible(lower, interval, offset);

            while (currentValue <= lastTickValue) {
                const float currentPosition = mapf(currentValue, lower, upper, 0.f, 1.f);

                positions.push_back(currentPosition);

                currentValue += interval;
            }
        }

        return positions;
    }
};
