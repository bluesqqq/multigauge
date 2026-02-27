#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/DisplayValue.h>
#include <multigauge/gauge/ticks/TickList.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>

class CircularScale : public CircularElement {
    private:
        TickList ticks;

        float radius = 1.0f;

        // Editable props list
        MG_EDITABLE_BEGIN()
            MG_PROP(radius)
        MG_EDITABLE_END()

    public:
        explicit CircularScale(Element* parent);

        CircularScale(Element* parent, const rapidjson::Value::ConstObject json);

        void draw(Graphics& g) const override;

        void update(int deltaTime) override;
};
REGISTER_ELEMENT_TYPE("circular-scale", CircularScale);