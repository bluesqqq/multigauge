#pragma once

#include <multigauge/graphics/colors/Color.h>
#include <multigauge/graphics/colors/ColorTimeline.h>

class ValueColor : public Color {
    MG_EDITOR_NAME("Value Color")
    MG_TYPE_ID("value")

    private:
        ColorTimeline timeline;
        Value* value = nullptr;
        std::string id; ///< reference string used during JSON serialization

    public:
        ValueColor() = default;

        ValueColor(Value* value, ColorTimeline timeline);

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

        // internal helper called after deserialization to resolve `id` -> pointer
        void updateValueId() { value = id.empty() ? nullptr : Value::find(id.c_str()); }

        MG_PROPS_PARENT(Color)

        MG_PROPS_BEGIN()
            MG_PROP(timeline, "timeline", "Timeline", "Color timeline.", "Timeline", "Animation")
            MG_PROP_CALLBACK(id, "id", "ID", "Value ID.", "Data", "Source", &ValueColor::updateValueId)
        MG_PROPS_END()
};

