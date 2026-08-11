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
    MG_PROP(backgroundColor_, "bgColor", "Background Color", "Color of the background.")
    MG_PROP(groundColor_, "groundColor", "Ground Color", "Color of the ground.")
    MG_PROP(horizonColor_, "horizonColor", "Horizon Color", "Color of the horizon.")
    MG_PROP(borderColor_, "borderColor", "Border Color", "Color of the border.")
    MG_PROPS_END()
};
} // namespace mg::gauge
