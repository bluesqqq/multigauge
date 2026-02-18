#pragma once

#include "color.h"

//----------[ KEYFRAME ]----------//

struct ColorKeyframe {
    /// @brief Position in the timeline
    float position;
    /// @brief The color at this position
    std::unique_ptr<Color> color;

    ColorKeyframe(std::unique_ptr<Color> color, float position);

    ColorKeyframe(const rapidjson::Value::ConstObject json);

    ColorKeyframe(const ColorKeyframe& other);

    ColorKeyframe& operator=(const ColorKeyframe& other);

    ColorKeyframe(ColorKeyframe&&) = default;
    
    ColorKeyframe& operator=(ColorKeyframe&&) = default;
};

//----------[ TIMELINE ]----------//

class ColorTimeline {
    private:
        std::vector<ColorKeyframe> keyframes;

        /// @brief Finds the index of the keyframe at or before the given position.
        /// @param position The position to search for
        /// @return The index of the keyframe
        size_t getKeyframeIndexAtPosition(float position) const;

    public:
        ColorTimeline();

        ColorTimeline(rgba color);

        ColorTimeline(const rapidjson::Value& json);

        ColorTimeline(const ColorTimeline& other);

        ColorTimeline& operator=(const ColorTimeline& other);

        ColorTimeline(ColorTimeline&&) = default;

        ColorTimeline& operator=(ColorTimeline&&) = default;

        //----------[ MUTATION ]----------//

        /// @brief Removes all keyframes from the timeline
        void clear();

        /// @brief Adds a StaticColor color keyframe to the timeline
        /// @param color The 16-bit color value
        /// @param position The position in the timeline
        void addKeyframe(rgba color, float position);

        /// @brief Adds a color keyframe to the timeline.
        /// @param color The color object for this keyframe
        /// @param position The position in the timeline
        void addKeyframe(std::unique_ptr<Color> color, float position);

        /// @brief Adds a color keyframe to the timeline.
        /// @param keyframe The ColorKeyframe to add
        void addKeyframe(ColorKeyframe keyframe);

        //----------[ QUERIES ]----------//

        /// @brief Gets the number of keyframes in the timeline.
        /// @return The number of keyframes
        size_t size() const;

        bool empty() const;

        /// @brief Gets the position of the first keyframe.
        /// @return The start position, or 0.0 if timeline is empty
        float getStartPosition() const;

        /// @brief Gets the position of the last keyframe.
        /// @return The start position, or 0.0 if timeline is empty
        float getEndPosition() const;

        //----------[ COLOR ]----------//

        /// @brief Gets the interpolated color at a specific position.
        /// @param position The position in the timeline
        /// @return The 16-bit color value at that position
        rgba getColor(float position) const;

        /// @brief Samples the timeline at regular intervals.
        /// @param startPosition The starting position for sampling
        /// @param endPosition The ending position for sampling
        /// @param numSamples The number of samples to take
        /// @return Vector of 16-bit color values
        std::vector<rgba> sample(float startPosition, float endPosition, size_t numSamples);

        //----------[ BLENDING ]----------//

        /// @brief Blends this timeline with a static color value.
        /// @param color The 16-bit color value to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = blend color)
        /// @return A new ColorTimeline object with the blended result
        ColorTimeline blended(rgba color, float alpha) const;

        /// @brief Blends this timeline with another Color object.
        /// @param other The Color object to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = blend color)
        /// @return A new ColorTimeline object with the blended result
        ColorTimeline blended(const Color& other, float alpha) const;

        /// @brief Blends this timeline with another ColorTImeline object.
        /// @param other The ColorTImeline object to blend with
        /// @param alpha The blend amount (0.0 = this color, 1.0 = blend color)
        /// @return A new ColorTimeline object with the blended result
        ColorTimeline blended(const ColorTimeline& other, float alpha) const;

        //----------[ UTILS ]----------//
        
        /// @brief Gets all keyframe positions in the timeline.
        /// @return Vector of all position values
        std::vector<float> getPositions() const;

        /// @brief Gets keyframe positions remapped to a new range
        /// @param start The new start position
        /// @param end The new end position
        /// @return Vector of remapped position values
        std::vector<float> getPositionsMapped(float start = 0.0f, float end = 1.0f) const;
};

//----------[ FILL STROKE TIMELINE ]----------//

struct FillStrokeTimeline {
    ColorTimeline fill;
    ColorTimeline stroke;
    float thickness = 1.0f;

    FillStrokeTimeline() = default;

    FillStrokeTimeline(ColorTimeline fill, ColorTimeline stroke, float thickness);

    FillStrokeTimeline(const rapidjson::Value::ConstObject json);

    //----------[ BLENDING ]----------//

    FillStrokeTimeline blended(rgba color, float alpha) const;
    FillStrokeTimeline blended(const Color& color, float alpha) const;
    FillStrokeTimeline blended(const ColorTimeline& color, float alpha) const;
    FillStrokeTimeline blended(const FillStrokeTimeline& other, float alpha) const;

    FillStroke getFillStrokeAtPosition(float position) const;
};