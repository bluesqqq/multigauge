#pragma once

#include <optional>

#include "TickStyle.h"
#include "utils.h"

struct SubTick {
    int divisions = 1;

    std::optional<float> length;
    std::optional<float> thickness;

    std::optional<TickStyle> style;
    std::optional<FillStrokeTimeline> color;

    std::optional<TickValueStyle> valueStyle;

    SubTick(const rapidjson::Value::ConstObject& json) {
        setInt(json, "divisions", divisions);
        setOptFloat(json, "length", length);
        setOptFloat(json, "thickness", thickness);
        setOptObj(json, "color", color);
        setOptObj(json, "valueStyle", valueStyle);
    }

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