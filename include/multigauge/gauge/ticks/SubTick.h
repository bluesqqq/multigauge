#pragma once

#include <multigauge/gauge/ticks/TickStyle.h>
#include <multigauge/properties/meta/Inspector.h>
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
    MG_PROP(divisions, "divisions")
    MG_PROP(length, "length")
    MG_PROP(thickness, "thickness")
    MG_PROP(paint, "paint")
    MG_PROPS_END()

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Subticks", {
        MG_PROPERTY("divisions", "Divisions", "number");
        MG_PROPERTY("length", "Length", "number");
        MG_PROPERTY("thickness", "Thickness", "number");
    });
    MG_INSPECTOR_END()
#endif

public:
};
} // namespace mg::gauge
