#pragma once

#include <multigauge/graphics/geometry/Line.h>

namespace mg {

template <typename T>
struct Rect {
    T x{};
    T y{};
    T width{};
    T height{};

    constexpr Rect() = default;
    constexpr Rect(T xValue, T yValue, T widthValue, T heightValue)
        : x(xValue), y(yValue), width(widthValue), height(heightValue) {}
    constexpr Rect(const Point<T>& position, T widthValue, T heightValue)
        : Rect(position.x, position.y, widthValue, heightValue) {}

    constexpr T getTop() const { return y; }
    constexpr T getBottom() const { return static_cast<T>(y + height); }
    constexpr T getLeft() const { return x; }
    constexpr T getRight() const { return static_cast<T>(x + width); }
    constexpr T getCenterX() const { return static_cast<T>(x + width / 2); }
    constexpr T getCenterY() const { return static_cast<T>(y + height / 2); }
    constexpr int getRightPixel() const { return static_cast<int>(getRight()) - 1; }
    constexpr int getBottomPixel() const { return static_cast<int>(getBottom()) - 1; }

    constexpr Point<T> getTopLeft() const { return {x, y}; }
    constexpr Point<T> getCenter() const { return {getCenterX(), getCenterY()}; }

    constexpr void setTop(T top) { height = static_cast<T>(getBottom() - top); y = top; }
    constexpr void setBottom(T bottom) { height = static_cast<T>(bottom - y); }
    constexpr void reduce(T amount) { x = static_cast<T>(x + amount); y = static_cast<T>(y + amount); width = static_cast<T>(width - 2 * amount); height = static_cast<T>(height - 2 * amount); }

    constexpr Rect<int> toInt() const { return {static_cast<int>(x), static_cast<int>(y), static_cast<int>(width), static_cast<int>(height)}; }
    constexpr Rect<float> toFloat() const { return {static_cast<float>(x), static_cast<float>(y), static_cast<float>(width), static_cast<float>(height)}; }
};

template <typename T>
inline Point<T> Point<T>::getAnchored(const Rect<T>& rectangle, Anchor anchor) {
    return getAnchored(rectangle.x, rectangle.y, rectangle.width, rectangle.height, anchor);
}

extern template struct Rect<int>;
extern template struct Rect<float>;

} // namespace mg
