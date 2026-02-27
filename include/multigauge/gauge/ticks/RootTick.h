#pragma once

#include <multigauge/gauge/ticks/TickStyle.h>
#include <multigauge/utils.h>

#include <cmath>
#include <vector>
#include <optional>

struct RootTick : public Editable {
    int divisions = 2;
    float interval = 1.0f;
    bool useDivisions = true;

    float length = 0;
    float thickness= 0;

    TickStyle style = TickStyle::LINE;
    PaintTimeline color;

    std::optional<TickValueStyle> valueStyle;

    MG_EDITABLE_BEGIN()
        MG_PROP(divisions)
        MG_PROP(interval)
        MG_PROP(useDivisions)
        MG_PROP(length)
        MG_PROP(thickness)
        MG_PROP(color)
        // TODO: need to do valueStyle
    MG_EDITABLE_END()

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