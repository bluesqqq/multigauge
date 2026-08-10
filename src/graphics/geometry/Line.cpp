#include <multigauge/graphics/geometry/Rect.h>

#include <algorithm>

namespace mg {

template <typename T>
std::optional<Line<T>> Line<T>::intersection(const Rect<T>& rectangle) const {
    float start = 0.0f;
    float end = 1.0f;
    const float dx = static_cast<float>(p2.x - p1.x);
    const float dy = static_cast<float>(p2.y - p1.y);
    const float p[] = {-dx, dx, -dy, dy};
    const float q[] = {static_cast<float>(p1.x - rectangle.getLeft()), static_cast<float>(rectangle.getRight() - p1.x), static_cast<float>(p1.y - rectangle.getTop()), static_cast<float>(rectangle.getBottom() - p1.y)};

    for (int index = 0; index != 4; ++index) {
        if (p[index] == 0.0f) {
            if (q[index] < 0.0f) return std::nullopt;
            continue;
        }
        const float ratio = q[index] / p[index];
        if (p[index] < 0.0f) start = std::max(start, ratio);
        else end = std::min(end, ratio);
        if (start > end) return std::nullopt;
    }

    return Line{
        static_cast<T>(p1.x + start * dx), static_cast<T>(p1.y + start * dy),
        static_cast<T>(p1.x + end * dx), static_cast<T>(p1.y + end * dy),
    };
}

template struct Line<int>;
template struct Line<float>;

} // namespace mg
