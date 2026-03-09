#pragma once

#include <multigauge/graphics/colors/Color.h>

class StaticColor : public Color {
    MG_TYPE_ID("static")

    private:
        rgba color;

        MG_PROPS_BEGIN()
            MG_PROP(color, "color", "Color", "RGBA color value.")
        MG_PROPS_END()
    
    public:
        /// @brief Constructs a StaticColor with default color
        StaticColor();

        StaticColor(rgba color);

        OwnedColor clone() const override;

        /// @brief Gets the static color value.
        /// @return The 16-bit color value
        rgba getColor() const override;

        /// @brief Gets the type of this color.
        /// @return Type::Static
        Type getType() const override;

        /// @brief Blends this color with a static color value.
        /// @param color The 16-bit color value to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = blend color)
        /// @return A new StaticColor object with the blended result
        OwnedColor blended(rgba color, float alpha) const override;

        /// @brief Blends this color with another Color object.
        /// @param color The Color object to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = other color)
        /// @return A new Color object of the same derived type as the input (e.g., blending with a TimeColor returns a TimeColor)
        OwnedColor blended(const Color& other, float alpha) const override;

};