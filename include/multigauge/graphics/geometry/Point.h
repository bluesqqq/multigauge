#pragma once

#include <multigauge/graphics/geometry/alignment.h>

namespace mg {

template <typename T>
struct Rect;

template <typename T>
struct Point {
    T x{};
    T y{};

    constexpr Point() = default;
    constexpr Point(T xValue, T yValue) : x(xValue), y(yValue) {}

    static Point<float> getPointOnUnitCircle(float angle);
    static Point getAnchored(T x, T y, T width, T height, Anchor anchor);
    static Point getAnchored(const Rect<T>& rectangle, Anchor anchor);

    constexpr Point operator+(const Point& other) const { return {static_cast<T>(x + other.x), static_cast<T>(y + other.y)}; }
    constexpr Point operator*(float scale) const { return {static_cast<T>(x * scale), static_cast<T>(y * scale)}; }
    constexpr Point translated(T deltaX, T deltaY) const { return {static_cast<T>(x + deltaX), static_cast<T>(y + deltaY)}; }
    constexpr Point<int> toInt() const { return {static_cast<int>(x), static_cast<int>(y)}; }
    constexpr Point<float> toFloat() const { return {static_cast<float>(x), static_cast<float>(y)}; }
};

extern template struct Point<int>;
extern template struct Point<float>;

} // namespace mg
