#include <multigauge/graphics/geometry/Rect.h>

#include <cmath>

namespace mg {

template <typename T>
Point<float> Point<T>::getPointOnUnitCircle(float angle) {
    return {std::cos(angle), std::sin(angle)};
}

template <typename T>
Point<T> Point<T>::getAnchored(T x, T y, T width, T height, Anchor anchor) {
    switch (anchor) {
        case Anchor::TopLeft: return {x, y};
        case Anchor::TopCenter: return {static_cast<T>(x - width / 2), y};
        case Anchor::TopRight: return {static_cast<T>(x - width), y};
        case Anchor::CenterLeft: return {x, static_cast<T>(y - height / 2)};
        case Anchor::Center: return {static_cast<T>(x - width / 2), static_cast<T>(y - height / 2)};
        case Anchor::CenterRight: return {static_cast<T>(x - width), static_cast<T>(y - height / 2)};
        case Anchor::BottomLeft: return {x, static_cast<T>(y - height)};
        case Anchor::BottomCenter: return {static_cast<T>(x - width / 2), static_cast<T>(y - height)};
        case Anchor::BottomRight: return {static_cast<T>(x - width), static_cast<T>(y - height)};
    }
    return {x, y};
}

template struct Point<int>;
template struct Point<float>;

} // namespace mg
