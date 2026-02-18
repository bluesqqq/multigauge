#pragma once

#include <optional>
#include "graphics/DisplayValue.h"
#include "graphics/gauge/Element.h"

class CircularElement : public Element {
    protected:
        std::optional<DisplayValue> value;
        std::optional<float> startAngle;
        std::optional<float> endAngle;

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

        CircularElement(Element* parent, const rapidjson::Value::ConstObject json) : Element(parent, json) {
            if (!json.HasMember("props") || !json["props"].IsObject()) return;
            const rapidjson::Value::ConstObject props = json["props"].GetObject();

            setOptFloat(props, "startAngle", startAngle);
            setOptFloat(props, "endAngle", endAngle);
            setOptObj<DisplayValue>(props, "value", value);
        }
        
        Type getType() const override { return Type::Circular; }
};

REGISTER_ELEMENT_TYPE("circular-element", CircularElement);