#pragma once

#include <optional>
#include <multigauge/graphics/DisplayValue.h>
#include <multigauge/gauge/Element.h>

class CircularElement : public Element {
    MG_EDITOR_NAME("Circular Element")
    protected:
        std::optional<DisplayValue> value;
        std::optional<float> startAngle;
        std::optional<float> endAngle;

        MG_EDITABLE_BEGIN()
            MG_EDITABLE_PROP_META(value, "value", "Value", "Value to display. Make null to inherit from parent.")
            MG_EDITABLE_PROP_META(startAngle, "startAngle", "Start Angle", "Angle to start from. Make null to inherit from parent.")
            MG_EDITABLE_PROP_META(endAngle, "endAngle", "End Angle", "Angle to end at. Make null to inherit from parent.")
        MG_EDITABLE_END()

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