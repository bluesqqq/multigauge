#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/gauge/ticks/TickList.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>

namespace mg::gauge {

class CircularScale : public CircularElement {
        MG_EDITOR_NAME("Circular Scale")
    MG_TYPE_ID("circular-scale")
    private:
        TickList ticks;

        float radius = 1.0f;

        MG_PROPS_PARENT(CircularElement)

        MG_PROPS_BEGIN()
    MG_PROP(ticks, "ticks", "Ticks", "List of ticks to draw.")
    MG_PROP(radius, "radius", "Radius", "Radius of the scale.")
        MG_PROPS_END()

    public:
        using CircularElement::CircularElement;

        void draw(Graphics& g) const override;

        void update(std::chrono::microseconds delta) override;
};

} // namespace mg::gauge
