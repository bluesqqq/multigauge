#pragma once

#include "color.h"
#include "ColorTimeline.h"

class ValueColor : public Color {
    private:
        ColorTimeline timeline;
        Value* value;

    public:
        ValueColor(Value* value);

        ValueColor(Value* value, ColorTimeline timeline);

        ValueColor(const rapidjson::Value::ConstObject json);

        ValueColor(const ValueColor& other);
        
        ValueColor& operator=(const ValueColor& other);

        std::unique_ptr<Color> clone() const override;

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
        std::unique_ptr<Color> blended(rgba color, float alpha) const override;

        /// @brief Blends this color with another Color object.
        /// @param color The Color object to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = other color)
        /// @return A new Color object with the blended result (will always be a ValueColor)
        std::unique_ptr<Color> blended(const Color& color, float alpha) const override;
};