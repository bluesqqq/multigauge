#pragma once

#include <multigauge/graphics/gauge/Element.h>
#include <multigauge/graphics/DisplayValue.h>
#include <multigauge/graphics/ticks/TickList.h>
#include <multigauge/graphics/gauge/elements/circular/CircularElement.h>

class CircularScale : public CircularElement {
    private:
        TickList ticks;

        float radius = 1.0f;

    public:
        explicit CircularScale(Element* parent);

        CircularScale(Element* parent, const rapidjson::Value::ConstObject json);

        void draw(Graphics& g) const override;

        void update(int deltaTime) override;
};
