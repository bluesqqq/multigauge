#pragma once

#include <optional>

#include <multigauge/graphics/geometry/Point.h>

namespace mg {

template <typename T>
struct Rect;

template <typename T>
struct Line {
    Point<T> p1;
    Point<T> p2;

    constexpr Line() = default;
    constexpr Line(T x1, T y1, T x2, T y2) : p1(x1, y1), p2(x2, y2) {}
    constexpr Line(const Point<T>& start, const Point<T>& end) : p1(start), p2(end) {}

    std::optional<Line> intersection(const Rect<T>& rectangle) const;
    constexpr Line<int> toInt() const { return {p1.toInt(), p2.toInt()}; }
    constexpr Line<float> toFloat() const { return {p1.toFloat(), p2.toFloat()}; }
};

extern template struct Line<int>;
extern template struct Line<float>;

} // namespace mg
