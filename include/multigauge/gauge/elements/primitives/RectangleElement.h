#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

namespace mg::gauge {

/// @brief Draws a filled rounded rectangle element.
class RectangleElement final : public Element {
    MG_EDITOR_NAME("Rectangle")
    MG_TYPE_ID("rectangle")

public:
    /// @brief Creates a rectangle element.
    RectangleElement() : Element(staticTypeId()) {}

    /// @brief Draws the rectangle in its layout bounds.
    void draw(::mg::graphics::Graphics&, const ::mg::Rect<float>&) const override;

private:
    ::mg::graphics::Paint paint_;
    float radius_ = 0.0f;

    MG_PROPS_PARENT(Element)
    MG_PROPS_BEGIN()
    MG_PROP(paint_, "paint")
    MG_PROP(radius_, "radius")
    MG_PROPS_END()

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Appearance", {
        MG_PROPERTY("paint.fill", "Fill", widget::color);
        MG_PROPERTY("paint.stroke", "Stroke", widget::color);
        MG_PROPERTY("paint.thickness", "Stroke Width", widget::number);
        MG_PROPERTY("radius", "Corner Radius", widget::number);
    });
    MG_INCLUDE("layout");
    MG_INSPECTOR_END()
#endif
};

} // namespace mg::gauge
