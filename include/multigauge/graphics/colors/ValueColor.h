#pragma once

#include <multigauge/graphics/colors/ColorTimeline.h>
#include <multigauge/value/ValueRef.h>

namespace mg::graphics {

class ValueColor final : public Color {
    MG_EDITOR_NAME("Value Color")
    MG_TYPE_ID("value")

    ColorTimeline timeline;
    ::mg::ValueRef value;

    MG_PROPS_PARENT(Color)
    MG_PROPS_BEGIN()
    MG_PROP(timeline, "timeline", "Gradient", "Normalized value-driven gradient.")
    MG_PROP(value, "id", "ID", "Value ID.")
    MG_PROPS_END()

public:
    ValueColor() = default;
    ValueColor(::mg::ValueRef value, ColorTimeline timeline);
    ValueColor(const ValueColor&) = default;
    ValueColor& operator=(const ValueColor&) = default;
    OwnedColor clone() const override;

protected:
    rgba resolveUncached(const ColorResolver::Frame& frame) const noexcept override;
};

} // namespace mg::graphics
