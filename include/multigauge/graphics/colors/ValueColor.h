#pragma once

#include <multigauge/graphics/colors/Color.h>
#include <multigauge/graphics/colors/ColorTimeline.h>
#include <multigauge/values/ValueRef.h>

namespace mg::graphics {

class ValueColor : public Color {
    MG_EDITOR_NAME("Value Color")
    MG_TYPE_ID("value")

    private:
        ColorTimeline timeline;
        ::mg::ValueRef value;

    public:
        ValueColor() = default;

        ValueColor(::mg::Value* value, ColorTimeline timeline);

        ValueColor(const ValueColor& other);
        
        ValueColor& operator=(const ValueColor& other);

        OwnedColor clone() const override;

        /// @brief Gets the current color value on the timeline.
        /// @return The current 16-bit color value
        rgba getColor() const override;

        /// @brief Gets the type of this color.
        /// @return Type::Value
        Type getType() const override;

        /// @brief Gets the associated color timeline
        /// @return Pointer to the ColorTimeline
        const ColorTimeline* getTimeline() const override;

        /// @brief Blends this color with a static color value.
        /// @param color The 16-bit color value to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = blend color)
        /// @return A new ValueColor object with the blended result
        OwnedColor blended(rgba color, float alpha) const override;

        /// @brief Blends this color with another Color object.
        /// @param color The Color object to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = other color)
        /// @return A new Color object with the blended result (will always be a ValueColor)
        OwnedColor blended(const Color& color, float alpha) const override;

        MG_PROPS_PARENT(Color)

        MG_PROPS_BEGIN()
    MG_PROP(timeline, "timeline", "Gradient", "Value-driven color gradient.")
    MG_PROP(value, "id", "ID", "Value ID.")
        MG_PROPS_END()
};

} // namespace mg::graphics

