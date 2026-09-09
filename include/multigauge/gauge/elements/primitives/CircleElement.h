#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

namespace mg::gauge {

/// @brief Draws a filled circle element.
class CircleElement final : public Element {
    MG_EDITOR_NAME("Circle")
    MG_TYPE_ID("circle")

public:
    /// @brief Creates a circle element.
    CircleElement() : Element(staticTypeId()) {}

    /// @brief Draws the circle in its layout bounds.
    void draw(::mg::graphics::Graphics&, const ::mg::Rect<float>&) const override;

private:
    ::mg::graphics::Paint paint_;

    MG_PROPS_PARENT(Element)
    MG_PROPS_BEGIN()
    MG_PROP(paint_, "paint")
    MG_PROPS_END()

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Appearance", {
        MG_PROPERTY("paint.fill", "Fill", "color");
        MG_PROPERTY("paint.stroke", "Stroke", "color");
        MG_PROPERTY("paint.thickness", "Stroke Width", "number");
    });
    MG_INCLUDE("layout");
    MG_INSPECTOR_END()
#endif
};

} // namespace mg::gauge
