#pragma once

#include <optional>
#include <multigauge/graphics/DisplayValue.h>
#include <multigauge/gauge/Element.h>

class CircularElement : public Element {
        MG_EDITOR_NAME("Circular Element")
    MG_TYPE_ID("circular-element")
    protected:
        std::optional<DisplayValue> value;
        std::optional<float> startAngle;
        std::optional<float> endAngle;

        MG_PROPS_PARENT(Element)

        MG_PROPS_BEGIN()
            MG_PROP(value, "value", "Value", "Value to display. Make null to inherit from parent.", "group", "Data", "Source")
            MG_PROP(startAngle, "startAngle", "Start Angle", "Angle to start from. Make null to inherit from parent.", "number", "Geometry", "Angles")
            MG_PROP(endAngle, "endAngle", "End Angle", "Angle to end at. Make null to inherit from parent.", "number", "Geometry", "Angles")
        MG_PROPS_END()

        const CircularElement* parentCircular() const {
            const Element* p = getParent();           // assumes you have a const getParent()
            if (!p) return nullptr;
            if (p->getType() != Element::Type::Circular) return nullptr;
            return static_cast<const CircularElement*>(p);
        }

        DisplayValue resolvedDisplayValue() const {
            if (value) return *value;
            if (auto p = parentCircular(); p && p->value) return *p->value;
            return DisplayValue{};
        }

        float resolvedStartAngle() const {
            if (startAngle) return *startAngle;
            if (auto p = parentCircular(); p && p->startAngle) return *p->startAngle;
            return 0.0f;
        }

        float resolvedEndAngle() const {
            if (endAngle) return *endAngle;
            if (auto p = parentCircular(); p && p->endAngle) return *p->endAngle;
            return 360.0f;
        }

    public:
        using Element::Element;
        
        Type getType() const override { return Type::Circular; }
};



