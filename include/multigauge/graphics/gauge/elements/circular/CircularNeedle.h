#pragma once

#include <multigauge/graphics/gauge/Element.h>
#include <multigauge/graphics/gauge/elements/circular/CircularElement.h>

class CircularNeedle : public CircularElement {
    private:
        Paint color;
        float radius = 1.0f;

    public:
        explicit CircularNeedle(Element* parent);

        CircularNeedle(Element* parent, const rapidjson::Value::ConstObject json);

        void draw(Graphics& g) const override;
};

REGISTER_ELEMENT_TYPE("circular-needle", CircularNeedle);