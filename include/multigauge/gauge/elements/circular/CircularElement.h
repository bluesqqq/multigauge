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
    MG_PROP(value_, "value")
    MG_PROP(startAngle_, "startAngle")
    MG_PROP(endAngle_, "endAngle")
    MG_PROPS_END()

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Circular", {
        MG_PROPERTY("value", "Value", "value-ref");
        MG_ROW({
            MG_PROPERTY("startAngle", "Start Angle", "number");
            MG_PROPERTY("endAngle", "End Angle", "number");
        });
    });
    MG_INCLUDE("layout");
    MG_INSPECTOR_END()
#endif
};

} // namespace mg::gauge
