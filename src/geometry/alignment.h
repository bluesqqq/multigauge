#pragma once
#include <utility>

enum class Direction {
    Right,
    Down,
    Left,
    Up
};

enum class Anchor {
    TopLeft,
    TopCenter,
    TopRight,
    CenterLeft,
    Center,
    CenterRight,
    BottomLeft,
    BottomCenter,
    BottomRight
};

enum LengthAlignment {
    OUTER = -1,
    CENTER = 0,
    INNER = 1
};

std::pair<float, float> alignLength(float start, float length, LengthAlignment alignment);