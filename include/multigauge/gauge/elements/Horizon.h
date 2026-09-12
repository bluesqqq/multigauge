#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

namespace mg::gauge {
/// @brief Draws an artificial horizon element.
class Horizon final : public Element {
    MG_EDITOR_NAME("Horizon")
    MG_TYPE_ID("horizon")
public:
    /// @brief Creates a horizon element.
    Horizon() : Element(staticTypeId()) {}

    /// @brief Draws the horizon in its layout bounds.
    void draw(::mg::graphics::Graphics&, const ::mg::Rect<float>&) const override;

    /// @brief Advances the horizon state.
    void update(std::chrono::microseconds) override;

private:
    int horizonDensityVertical_ = 25;
    int horizonDensityHorizontal_ = 12;
    int horizonVAngle_ = 8;
    int horizonHAngle_ = 8;
    float zPosition_ = 0.0f;
    float xPosition_ = 0.0f;
    ::mg::graphics::OwnedColor backgroundColor_, groundColor_, horizonColor_, borderColor_;
    MG_PROPS_PARENT(Element)
    MG_PROPS_BEGIN()
    MG_PROP(backgroundColor_, "bgColor")
    MG_PROP(groundColor_, "groundColor")
    MG_PROP(horizonColor_, "horizonColor")
    MG_PROP(borderColor_, "borderColor")
    MG_PROPS_END()

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Colors", {
        MG_PROPERTY("bgColor", "Background", widget::color);
        MG_PROPERTY("groundColor", "Ground", widget::color);
        MG_PROPERTY("horizonColor", "Horizon", widget::color);
        MG_PROPERTY("borderColor", "Border", widget::color);
    });
    MG_INCLUDE("layout");
    MG_INSPECTOR_END()
#endif
};
} // namespace mg::gauge
