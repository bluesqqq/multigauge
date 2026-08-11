#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/value/ValueView.h>
#include <optional>

namespace mg::gauge {

/// @brief Base class for elements with inherited circular value and angles.
class CircularElement : public Element {
    MG_EDITOR_NAME("Circular Element")
    MG_TYPE_ID("circular-element")

public:
    /// @brief Creates a circular element with a serialized type identifier.
    explicit CircularElement(std::string_view typeId = staticTypeId()) : Element(typeId) {}

protected:
    ::mg::ValueView resolvedValueView() const;
    float resolvedStartAngle() const;
    float resolvedEndAngle() const;

private:
    friend class GaugeFace;

    void resolveInherited(::mg::ValueView value, float startAngle, float endAngle) noexcept;

    std::optional<::mg::ValueView> value_;
    std::optional<float> startAngle_;
    std::optional<float> endAngle_;
    ::mg::ValueView resolvedValue_;
    float resolvedStartAngle_ = 0.0f;
    float resolvedEndAngle_ = 360.0f;

    MG_PROPS_PARENT(Element)
    MG_PROPS_BEGIN()
    MG_PROP(value_, "value", "Value", "Value to display. Make null to inherit from parent.")
    MG_PROP(startAngle_,
            "startAngle",
            "Start Angle",
            "Angle to start from. Make null to inherit from parent.")
    MG_PROP(endAngle_,
            "endAngle",
            "End Angle",
            "Angle to end at. Make null to inherit from parent.")
    MG_PROPS_END()
};

} // namespace mg::gauge
