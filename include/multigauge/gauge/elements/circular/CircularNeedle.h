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
    MG_PROP(paint_, "paint", "Paint", "Paint options for the needle.")
    MG_PROP(radius_, "radius", "Radius", "Radius of the needle.")
    MG_PROPS_END()
};

} // namespace mg::gauge
