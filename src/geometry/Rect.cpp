#include "Rect.h"
#include <algorithm>
#include <cmath>
#include <array>

template<typename T>
Rect<T>::Rect(T x, T y, T w, T h) : Rect(Point<T>(x, y), w, h) {}

template<typename T>
Rect<T>::Rect(const Point<T>& pos, T w, T h) : x(pos.x), y(pos.y), width(w), height(h) {}

template<typename T>
Rect<T> Rect<T>::fromPoints(Point<T> p1, Point<T> p2) { return Rect<T>(p1, p2.x - p1.x, p2.y - p1.y);}

template <typename T>
Point<T> Rect<T>::position() const { return Point<T>(x, y); }

template <typename T>
T Rect<T>::area() const { return width * height; }

template<typename T>
T Rect<T>::perimeter() const { return width + width + height + height; }

template<typename T>
bool Rect<T>::isEmpty() const { return width == 0 && height == 0; }

template<typename T>
bool Rect<T>::operator==(const Rect<T>& other) const {
    return x == other.x && y == other.y && width == other.width && height == other.height;
}

template<typename T>
bool Rect<T>::operator!=(const Rect<T>& other) const {
    return !(*this == other);
}

// INTERSECTIONS

template<typename T>
bool Rect<T>::contains(const Point<T>& point) const {
    return point.x >= getLeft() && point.x <= getRight() && point.y >= getTop() && point.y <= getBottom();
}

template<typename T>
bool Rect<T>::intersects(const Rect<T>& other) const {
    return (getRight() >= other.getLeft() && getLeft() <= other.getRight() && getBottom() >= other.getTop() && getTop() <= other.getBottom());
}

template<typename T>
std::optional<Rect<T>> Rect<T>::intersection(const Rect<T>& other) const {
    T left = std::max(getLeft(), other.getLeft());
    T top = std::max(getTop(), other.getTop());
    T right = std::min(getRight(), other.getRight());
    T bottom = std::min(getBottom(), other.getBottom());

    if (left < right && top < bottom) {
        return Rect<T>(Point<T>(left, top), right - left, bottom - top);
    }
    return std::nullopt;
}

template<typename T>
bool Rect<T>::intersects(const Line<T>& line) const { return line.intersects(*this); }

template<typename T>
std::optional<Line<T>> Rect<T>::intersection(const Line<T>& line) const { return line.intersection(*this); }

template <typename T>
bool Rect<T>::contains(const Line<T> &line) { return contains(line.p1) && contains(line.p2); }

/*
template<typename T>
void Rect<T>::clip(const Rect<T>& other) {
    auto opt = intersection(other);
    if (opt) {
        *this = *opt;
    } else {
        // no overlap -> make empty rectangle positioned at intersection coords
        T left = std::max(getLeft(), other.getLeft());
        T top = std::max(getTop(), other.getTop());
        position = Point<T>(left, top);
        width = 0;
        height = 0;
    }
}

template<typename T>
Rect<T> Rect<T>::clipped(const Rect<T>& other) const {
    auto opt = intersection(other);
    if (opt) return *opt;
    return Rect<T>(Point<T>(0,0), 0, 0);
}
    */

// POSITIONING

template<typename T>
void Rect<T>::setTop(T top) {
    T bottom = getBottom();
    y = top;
    height = bottom - top;
}

template<typename T>
void Rect<T>::setBottom(T bottom) {
    height = bottom - y;
}

template<typename T>
void Rect<T>::setLeft(T left) {
    T right = getRight();
    x = left;
    width = right - left;
}

template<typename T>
void Rect<T>::setRight(T right) {
    width = right - x;
}

template<typename T>
void Rect<T>::setCenterX(T centerX) {
    x = centerX - (width / (T)2);
}

template<typename T>
void Rect<T>::setCenterY(T centerY) {
    y = centerY - (height / (T)2);
}

template<typename T>
T Rect<T>::getTop() const { return y; }
template<typename T>
T Rect<T>::getBottom() const { return y + height; }
template<typename T>
T Rect<T>::getLeft() const { return x; }
template<typename T>
T Rect<T>::getRight() const { return x + width; }
template<typename T>
T Rect<T>::getCenterX() const { return getLeft() + (width / (T)2); }
template<typename T>
T Rect<T>::getCenterY() const { return getTop() + (height / (T)2); }

template <typename T>
int Rect<T>::getBottomPixel() const { return this->getBottom() - 1; }

template <typename T>
int Rect<T>::getRightPixel() const { return this->getRight() - 1; }

template<typename T>
Point<T> Rect<T>::getTopLeft() const { return Point<T>(x, y); }
template<typename T>
Point<T> Rect<T>::getBottomLeft() const { return Point<T>(x, y + height); }
template<typename T>
Point<T> Rect<T>::getTopRight() const { return Point<T>(x + width, y); }
template<typename T>
Point<T> Rect<T>::getBottomRight() const { return Point<T>(x + width, y + height); }
template<typename T>
Point<T> Rect<T>::getCenter() const { return Point<T>(x + width / (T)2, y + height / (T)2); }

template <typename T>
Point<int> Rect<T>::getBottomLeftPixel() const { return Point<int>((int)getLeft(), (int)getBottomPixel()); }

template <typename T>
Point<int> Rect<T>::getBottomRightPixel() const { return Point<int>((int)getRightPixel(), (int)getBottomPixel()); }

template <typename T>
Point<int> Rect<T>::getTopRightPixel() const { return Point<int>((int)getRightPixel(), (int)getTop()); }

template<typename T>
Line<T> Rect<T>::getTopEdge() const { return Line<T>(getTopLeft(), getTopRight()); }
template<typename T>
Line<T> Rect<T>::getBottomEdge() const { return Line<T>(getBottomRight(), getBottomLeft()); }
template<typename T>
Line<T> Rect<T>::getLeftEdge() const { return Line<T>(getBottomLeft(), getTopLeft()); }
template<typename T>
Line<T> Rect<T>::getRightEdge() const { return Line<T>(getTopRight(), getBottomRight()); }

template<typename T>
std::vector<Line<T>> Rect<T>::getEdges() const {
    std::vector<Line<T>> edges;
    edges.reserve(4);
    edges.push_back(getTopEdge());
    edges.push_back(getRightEdge());
    edges.push_back(getBottomEdge());
    edges.push_back(getLeftEdge());
    return edges;
}

// SHAPING

template<typename T>
void Rect<T>::trimTop(T amount) {
    y += amount;
    height -= amount;
}
template<typename T>
void Rect<T>::trimBottom(T amount) { height -= amount; }
template<typename T>
void Rect<T>::trimLeft(T amount) { x += amount; width -= amount; }
template<typename T>
void Rect<T>::trimRight(T amount) { width -= amount; }

template<typename T>
Rect<T> Rect<T>::trimmedTop(T amount) const {
    return Rect<T>(x, y + amount, width, height - amount);
}
template<typename T>
Rect<T> Rect<T>::trimmedBottom(T amount) const {
    return Rect<T>(x, y, width, height - amount);
}
template<typename T>
Rect<T> Rect<T>::trimmedLeft(T amount) const {
    return Rect<T>(x + amount, y, width - amount, height);
}
template<typename T>
Rect<T> Rect<T>::trimmedRight(T amount) const {
    return Rect<T>(x, y, width - amount, height);
}

template<typename T>
Rect<T> Rect<T>::removeFromTop(T amount) {
    Rect<T> removed(x, y, width, amount);
    y += amount;
    height -= amount;
    return removed;
}
template<typename T>
Rect<T> Rect<T>::removeFromBottom(T amount) {
    Rect<T> removed(x, y + height - amount, width, amount);
    height -= amount;
    return removed;
}
template<typename T>
Rect<T> Rect<T>::removeFromLeft(T amount) {
    Rect<T> removed(x, y, amount, height);
    x += amount;
    width -= amount;
    return removed;
}
template<typename T>
Rect<T> Rect<T>::removeFromRight(T amount) {
    Rect<T> removed(x + width - amount, y, amount, height);
    width -= amount;
    return removed;
}

// TRANSLATION

template<typename T>
Rect<T> Rect<T>::operator+(Point<T> delta) const { return translated(delta); }
template<typename T>
Rect<T>& Rect<T>::operator+=(Point<T> delta) { translate(delta); return *this; }
template<typename T>
Rect<T> Rect<T>::operator-(Point<T> delta) const { return translated(-delta); }
template<typename T>
Rect<T>& Rect<T>::operator-=(Point<T> delta) { translate(-delta); return *this; }

template<typename T>
void Rect<T>::translate(T deltaX, T deltaY) { x += deltaX; y += deltaY; }
template<typename T>
void Rect<T>::translate(Point<T> delta) { x += delta.x; y += delta.y; }
template<typename T>
Rect<T> Rect<T>::translated(T deltaX, T deltaY) const { return Rect<T>(x + deltaX, y + deltaY, width, height); }
template<typename T>
Rect<T> Rect<T>::translated(Point<T> delta) const { return Rect<T>(x + delta.x, y + delta.y, width, height); }

template<typename T>
void Rect<T>::interpolate(const Rect<T>& other, float t) {
    x = lerp(x, other.x, t);
    y = lerp(y, other.y, t);
    width = lerp(width, other.width, t);
    height = lerp(height, other.height, t);
}
template<typename T>
Rect<T> Rect<T>::interpolated(const Rect<T>& other, float t) const {
    return Rect<T>(lerp(x, other.x, t), lerp(y, other.y, t), lerp(width, other.width, t), lerp(height, other.height, t));
}

// SCALING

template<typename T>
Rect<T> Rect<T>::operator*(float scale) const { return Rect<T>(position() * scale, width * scale, height * scale); }
template<typename T>
Rect<T>& Rect<T>::operator*=(float scale) { auto pos = position() *= scale; x = pos.x; y = pos.y; width *= scale; height *= scale; return *this; }
template<typename T>
Rect<T> Rect<T>::operator/(float scale) const { return Rect<T>(position() / scale, width / scale, height / scale); }
template<typename T>
Rect<T>& Rect<T>::operator/=(float scale) { auto pos = position() /= scale; x = pos.x; y = pos.y; width /= scale; height /= scale; return *this; }

template<typename T>
void Rect<T>::scaleFromOrigin(float scale) { auto pos = position(); pos.scaleFromOrigin(scale); x = pos.x; y = pos.y; width *= scale; height *= scale; }
template<typename T>
Rect<T> Rect<T>::scaledFromOrigin(float scale) const { return Rect<T>(position().scaledFromOrigin(scale), width * scale, height * scale); }

template<typename T>
void Rect<T>::scaleFromPoint(const Point<T>& other, float scale) { auto pos = position(); pos.scaleFromPoint(other, scale); x = pos.x, y = pos.y; width *= scale; height *= scale; }
template<typename T>
Rect<T> Rect<T>::scaledFromPoint(const Point<T>& other, float scale) const { return Rect<T>(position().scaledFromPoint(other, scale), width * scale, height * scale); }

template<typename T>
void Rect<T>::reduce(T deltaX, T deltaY) { x += deltaX; y += deltaY;  width -= (T)(2 * deltaX); height -= (T)(2 * deltaY); }
template<typename T>
void Rect<T>::reduce(T delta) { reduce(delta, delta); }

template<typename T>
Rect<T> Rect<T>::reduced(T deltaX, T deltaY) const { return Rect<T>(Point<T>(x + deltaX, y + deltaY), width - (T)(2 * deltaX), height - (T)(2 * deltaY)); }
template<typename T>
Rect<T> Rect<T>::reduced(T delta) const { return reduced(delta, delta); }

template<typename T>
void Rect<T>::expand(T deltaX, T deltaY) { x -= deltaX; y -= deltaY; width += (T)(2 * deltaX); height += (T)(2 * deltaY); }
template<typename T>
void Rect<T>::expand(T delta) { expand(delta, delta); }

template<typename T>
Rect<T> Rect<T>::expanded(T deltaX, T deltaY) const { return Rect<T>(x - deltaX, y - deltaY, width + (T)(2 * deltaX), height + (T)(2 * deltaY)); }
template<typename T>
Rect<T> Rect<T>::expanded(T delta) const { return expanded(delta, delta); }

// ROTATION helpers: compute bounding box of rotated corners
template<typename T>
static Rect<T> rectangleFromRotatedCorners(const std::array<Point<T>,4>& corners) {
    T minx = corners[0].x, maxx = corners[0].x;
    T miny = corners[0].y, maxy = corners[0].y;
    for (size_t i=1;i<4;++i) {
        minx = std::min(minx, corners[i].x);
        maxx = std::max(maxx, corners[i].x);
        miny = std::min(miny, corners[i].y);
        maxy = std::max(maxy, corners[i].y);
    }
    return Rect<T>(Point<T>(minx, miny), maxx - minx, maxy - miny);
}

template<typename T>
void Rect<T>::rotateAroundOrigin(float angle) {
    *this = rotatedAroundOrigin(angle);
}
template<typename T>
Rect<T> Rect<T>::rotatedAroundOrigin(float angle) const {
    float s = std::sin(angle);
    float c = std::cos(angle);
    auto tl = getTopLeft();
    auto tr = getTopRight();
    auto bl = getBottomLeft();
    auto br = getBottomRight();

    std::array<Point<T>,4> pts = { tl, tr, bl, br };
    std::array<Point<T>,4> rpts;
    for (int i=0;i<4;++i) {
        float x = static_cast<float>(pts[i].x);
        float y = static_cast<float>(pts[i].y);
        float rx = x * c - y * s;
        float ry = x * s + y * c;
        rpts[i] = Point<T>((T)rx, (T)ry);
    }
    return rectangleFromRotatedCorners(rpts);
}

template<typename T>
void Rect<T>::rotateAroundPoint(const Point<T>& other, float angle) {
    *this = rotatedAroundPoint(other, angle);
}
template<typename T>
Rect<T> Rect<T>::rotatedAroundPoint(const Point<T>& other, float angle) const {
    float s = std::sin(angle);
    float c = std::cos(angle);
    auto tl = getTopLeft();
    auto tr = getTopRight();
    auto bl = getBottomLeft();
    auto br = getBottomRight();

    std::array<Point<T>,4> pts = { tl, tr, bl, br };
    std::array<Point<T>,4> rpts;
    for (int i=0;i<4;++i) {
        float x = static_cast<float>(pts[i].x - other.x);
        float y = static_cast<float>(pts[i].y - other.y);
        float rx = x * c - y * s + other.x;
        float ry = x * s + y * c + other.y;
        rpts[i] = Point<T>((T)rx, (T)ry);
    }
    return rectangleFromRotatedCorners(rpts);
}

// REFLECTION

template<typename T>
void Rect<T>::reflectAcrossHorizontal(T horizontal) {
    *this = reflectedAcrossHorizontal(horizontal);
}
template<typename T>
Rect<T> Rect<T>::reflectedAcrossHorizontal(T horizontal) const {
    auto tl = getTopLeft();
    auto tr = getTopRight();
    auto bl = getBottomLeft();
    auto br = getBottomRight();
    std::array<Point<T>,4> pts = { tl, tr, bl, br };
    for (auto &p : pts) {
        p.y = (T)(2 * horizontal - p.y);
    }
    return rectangleFromRotatedCorners(pts);
}

template<typename T>
void Rect<T>::reflectAcrossVertical(T vertical) {
    *this = reflectedAcrossVertical(vertical);
}
template<typename T>
Rect<T> Rect<T>::reflectedAcrossVertical(T vertical) const {
    auto tl = getTopLeft();
    auto tr = getTopRight();
    auto bl = getBottomLeft();
    auto br = getBottomRight();
    std::array<Point<T>,4> pts = { tl, tr, bl, br };
    for (auto &p : pts) {
        p.x = (T)(2 * vertical - p.x);
    }
    return rectangleFromRotatedCorners(pts);
}

// CONVERSION

template<typename T>
Rect<float> Rect<T>::toFloat() const { return Rect<float>((float)x, (float)y, (float)width, (float)height); }
template<typename T>
Rect<int> Rect<T>::toInt() const { return Rect<int>((int)x, (int)y, (int)width, (int)height); }

template <typename T>
Path<T> Rect<T>::asPath() const { return Path<T>(*this); }

template struct Rect<int>;
template struct Rect<float>;