#pragma once

#include "color.h"

class UserColor : public Color {
    public:
        enum class Slot : uint8_t {
            Primary,
            Secondary,
            Background,
        };

    private:
        Slot slot;
        
        static rgba userColors[3];
    
    public:
        UserColor(Slot slot = Slot::Primary);

        UserColor(const rapidjson::Value::ConstObject json);

        std::unique_ptr<Color> clone() const override;
        
        /// @brief Gets the user-defined color value.
        /// @return The 16-bit color value
        rgba getColor() const override;

        /// @brief Gets the type of this color.
        /// @return Type::User
        Type getType() const override;

        /// @brief Blends this color with a static color value.
        /// @param color The 16-bit color value to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = blend color)
        /// @return A new StaticColor object with the blended result
        std::unique_ptr<Color> blended(rgba color, float alpha) const override;

        /// @brief Blends this color with another Color object.
        /// @param color The Color object to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = other color)
        /// @return A new Color object of the same derived type as the input (e.g., blending with a TimeColor returns a TimeColor)
        std::unique_ptr<Color> blended(const Color& other, float alpha) const override;
};