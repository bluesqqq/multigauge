#pragma once

#include <multigauge/graphics/colors/Color.h>
#include <multigauge/graphics/colors/ColorTimeline.h>

class TimeColor : public Color {
    MG_TYPE_ID("time")
    
    public:
        /// @brief Defines how the timeline loops
        enum class LoopType {
            /// @brief Start to end repeatedly
            Forward,
            /// @brief End to start repeatedly
            Reverse,
            /// @brief Alternates between forward and reverse
            PingPong
        };

    private:
        ColorTimeline timeline;
        LoopType loopType;

        MG_PROPS_BEGIN()
            MG_PROP(timeline, "timeline", "Timeline", "Color timeline.")
            MG_PROP(loopType, "loop", "Loop", "Type of looping to use.")
        MG_PROPS_END()

        /// @brief Retrieves the current time value.
        /// @return The current time position in milliseconds
        float getTime() const;

    public:
        /// @brief Constructs a TimeColor with the default timeline and loop type.
        TimeColor();

        /// @brief Constructs a TimeColor with a specified timeline and loop type.
        /// @param timeline The color timeline (position = time in milliseconds)
        /// @param loopType The looping mode
        TimeColor(ColorTimeline timeline, LoopType loopType = LoopType::Forward);

        TimeColor(const TimeColor& other);
        
        TimeColor& operator=(const TimeColor& other);

        OwnedColor clone() const override;

        /// @brief Gets the current color value on the timeline.
        /// @return The current 16-bit color value
        rgba getColor() const override;

        /// @brief Gets the type of this color.
        /// @return Type::Time
        Type getType() const override;

        /// @brief Gets the associated color timeline
        /// @return Pointer to the ColorTimeline
        const ColorTimeline* getTimeline() const override;

        /// @brief Blends this color with a static color value.
        /// @param color The 16-bit color value to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = blend color)
        /// @return A new TimeColor object with the blended result
        OwnedColor blended(rgba color, float alpha) const override;

        /// @brief Blends this color with another Color object.
        /// @param color The Color object to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = other color)
        /// @return A new Color object with the blended result (will always be a TimeColor)
        OwnedColor blended(const Color& other, float alpha) const override;
};

CODEC_BEGIN(TimeColor::LoopType)
    DECODE() {
        if (!v.IsString()) return false;
        const char* s = v.GetString();
        if (std::strcmp(s, "forward") == 0) out = TimeColor::LoopType::Forward;
        else if (std::strcmp(s, "reverse") == 0) out = TimeColor::LoopType::Reverse;
        else if (std::strcmp(s, "pingpong") == 0) out = TimeColor::LoopType::PingPong;
        else return false;
        return true;
    }

    ENCODE() {
        const char* s = nullptr;
        switch (v) {
            case TimeColor::LoopType::Forward:  s = "forward"; break;
            case TimeColor::LoopType::Reverse:  s = "reverse"; break;
            case TimeColor::LoopType::PingPong: s = "pingpong"; break;
        }
        if (s) out.SetString(s, a);
        else out.SetNull();
        return true;
    }
CODEC_END()