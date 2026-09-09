#pragma once

#include <multigauge/gauge/elements/circular/CircularElement.h>
#include <multigauge/graphics/colors/Color.h>

namespace mg::gauge {

/// @brief Draws a radial needle for a circular value.
class CircularNeedle final : public CircularElement {
    MG_EDITOR_NAME("Circular Needle")
    MG_TYPE_ID("circular-needle")

public:
    /// @brief Creates a circular needle element.
    CircularNeedle() : CircularElement(staticTypeId()) {}

    /// @brief Draws the needle in its layout bounds.
    void draw(::mg::graphics::Graphics&, const ::mg::Rect<float>&) const override;

private:
    ::mg::graphics::Paint paint_;
    float radius_ = 1.0f;

    MG_PROPS_PARENT(CircularElement)
    MG_PROPS_BEGIN()
    MG_PROP(paint_, "paint")
    MG_PROP(radius_, "radius")
    MG_PROPS_END()

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Circular", {
        MG_PROPERTY("value", "Value", "value-ref");
        MG_ROW({
            MG_PROPERTY("startAngle", "Start Angle", "number");
            MG_PROPERTY("endAngle", "End Angle", "number");
        });
    });
    MG_SECTION("Needle", {
        MG_PROPERTY("paint.fill", "Fill", "color");
        MG_PROPERTY("paint.stroke", "Stroke", "color");
        MG_PROPERTY("paint.thickness", "Stroke Width", "number");
        MG_PROPERTY("radius", "Radius", "number");
    });
    MG_INCLUDE("layout");
    MG_INSPECTOR_END()
#endif
};

} // namespace mg::gauge
