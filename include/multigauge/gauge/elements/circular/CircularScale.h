#pragma once

#include <multigauge/gauge/elements/circular/CircularElement.h>
#include <multigauge/gauge/ticks/TickList.h>

namespace mg::gauge {

/// @brief Draws a circular tick scale.
class CircularScale final : public CircularElement {
    MG_EDITOR_NAME("Circular Scale")
    MG_TYPE_ID("circular-scale")

public:
    /// @brief Creates a circular scale element.
    CircularScale() : CircularElement(staticTypeId()) {}

    /// @brief Draws the scale in its layout bounds.
    void draw(::mg::graphics::Graphics&, const ::mg::Rect<float>&) const override;

    /// @brief Advances the scale state.
    void update(std::chrono::microseconds) override;

private:
    ::mg::gauge::TickList ticks_;
    float radius_ = 1.0f;

    MG_PROPS_PARENT(CircularElement)
    MG_PROPS_BEGIN()
    MG_PROP(ticks_, "ticks", "Ticks", "List of ticks to draw.")
    MG_PROP(radius_, "radius", "Radius", "Radius of the scale.")
    MG_PROPS_END()
};

} // namespace mg::gauge
