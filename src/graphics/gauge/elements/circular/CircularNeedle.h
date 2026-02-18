#pragma once

#include "graphics/gauge/Element.h"
#include "CircularElement.h"

class CircularNeedle : public CircularElement {
    private:
        FillStroke color;
        float radius = 1.0f;

    public:
        explicit CircularNeedle(Element* parent);

        CircularNeedle(Element* parent, const rapidjson::Value::ConstObject json);

        void draw(Graphics& g) const override;
};
