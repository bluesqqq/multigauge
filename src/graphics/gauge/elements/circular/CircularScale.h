#pragma once

#include "graphics/gauge/Element.h"
#include "graphics/DisplayValue.h"
#include "graphics/ticks/TickList.h"
#include "CircularElement.h"

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
