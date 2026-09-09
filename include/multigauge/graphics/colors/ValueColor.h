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
    MG_PROP(timeline, "timeline")
    MG_PROP(value, "id")
    MG_PROPS_END()

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Value Color", {
        MG_PROPERTY("id", "Value ID", "text");
        MG_PROPERTY("timeline", "Stops", "gradient");
    });
    MG_INSPECTOR_END()
#endif

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
