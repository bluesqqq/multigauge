#include <multigauge/geometry/Point.h>
#include <multigauge/constants.h>
#include <cmath>
#include <algorithm>

#include <multigauge/geometry/Line.h>
#include <multigauge/geometry/Rect.h>
#include <multigauge/geometry/Circle.h>

template <typename T>
Point<T>::Point() : x(0), y(0) {}

template <typename T>
Point<T>::Point(T x, T y) : x(x), y(y) {}

template <typename T>
Point<float> Point<T>::getPointOnUnitCircle(float angle) {
    return Point<float>(cosf(angle), sinf(angle));
}

template <typename T>
Point<float> Point<T>::getPointOnCircumference(float radius, float angle) {
    return Point<float>(radius * cosf(angle), radius * sinf(angle));
}

template <typename T>
Point<float> Point<T>::getPointOnCircumference(float radiusX, float radiusY, float angle) {
    return Point<float>(radiusX * cosf(angle), radiusY * sinf(angle));
}

template <typename T>
Point<T> Point<T>::getAnchored(T x, T y, T width, T height, Anchor anchor) {
    switch (anchor) {
        case Anchor::TopLeft: return Point(x, y);
        case Anchor::TopCenter: return Point(x - width / 2, y);
        case Anchor::TopRight: return Point(x - width, y);
        case Anchor::CenterLeft: return Point(x, y - height / 2);
        case Anchor::Center: return Point(x - width / 2, y - height / 2);
        case Anchor::CenterRight: return Point(x - width, y - height / 2);
        case Anchor::BottomLeft: return Point(x, y - height);
        case Anchor::BottomCenter: return Point(x - width / 2, y - height);
        case Anchor::BottomRight: return Point(x - width, y - height);
        default: return Point(x, y);
    }
}

template <typename T>
Point<T> Point<T>::getAnchored(const Point<T>& point, T width, T height, Anchor anchor) {
    return Point<T>::getAnchored(point.x, point.y, width, height, anchor);
}

template <typename T>
Point<T> Point<T>::getAnchored(const Rect<T>& rectangle, Anchor anchor) {
    return Point<T>::getAnchored(rectangle.x, rectangle.y, rectangle.width, rectangle.height, anchor);
}

template <typename T>
inline bool Point<T>::operator==(const Point<T>& other) const { return x == other.x && y == other.y; }

template <typename T>
inline bool Point<T>::operator!=(const Point<T>& other) const { return !(*this == other); }

template <typename T>
inline Point<T> Point<T>::operator-() const { return Point<T>(-x, -y); }

template <typename T>
inline Point<T>& Point<T>::operator=(const Point<T>& other) { x = other.x; y = other.y; return *this; }
template <typename T>
inline Point<T>& Point<T>::operator+=(const Point<T>& other) { x += other.x; y += other.y; return *this; }
template <typename T>
inline Point<T>& Point<T>::operator-=(const Point<T>& other) { x -= other.x; y -= other.y; return *this; }
template <typename T>
inline Point<T>& Point<T>::operator*=(const Point<T>& other) { x *= other.x; y *= other.y; return *this; }
template <typename T>
inline Point<T>& Point<T>::operator/=(const Point<T>& other) { x /= other.x; y /= other.y; return *this; }

template <typename T>
inline Point<T>& Point<T>::operator*=(float scalar) { x *= scalar; y *= scalar; return *this; }
template <typename T>
inline Point<T>& Point<T>::operator/=(float scalar) { x /= scalar; y /= scalar; return *this; }

template <typename T>
inline Point<T> Point<T>::operator-(const Point<T>& other) const { return Point<T>(x - other.x, y - other.y); }
template <typename T>
inline Point<T> Point<T>::operator+(const Point<T>& other) const { return Point<T>(x + other.x, y + other.y); }
template <typename T>
inline Point<T> Point<T>::operator*(const Point<T>& other) const { return Point<T>(x * other.x, y * other.y); }
template <typename T>
inline Point<T> Point<T>::operator/(const Point<T>& other) const { return Point<T>(x / other.x, y / other.y); }

template <typename T>
inline Point<T> Point<T>::operator*(float scalar) const { return Point<T>(x * scalar, y * scalar); }
template <typename T>
inline Point<T> Point<T>::operator/(float scalar) const { return Point<T>(x / scalar, y / scalar); }

template <typename T>
inline Point<T> Point<T>::operator^(float exponent) const { return Point<T>(powf(x, exponent), powf(y, exponent)); }

template <typename T>
float Point<T>::magnitude() const { return std::sqrt(x * x + y * y); }

template <typename T>
Point<float> Point<T>::normalized() const {
    float mag = magnitude();
    return Point<float>(x / mag, y / mag);
}

template <typename T>
float Point<T>::distance() const { return std::sqrt(x * x + y * y); }
template <typename T>
float Point<T>::distanceTo(const Point<T>& other) const { return std::sqrt((other.x - x) * (other.x - x) + (other.y - y) * (other.y - y)); }
template <typename T>
float Point<T>::dot(const Point<T>& other) const { return x * other.x + y * other.y; }
template <typename T>
float Point<T>::cross(const Point<T>& other) const { return x * other.y - y * other.x; }

// ====== [INTERSECTIONS] ====== //

template <typename T>
bool Point<T>::isContainedBy(const Rect<T>& rectangle) { return rectangle.contains(*this); }
template <typename T>
bool Point<T>::isContainedBy(const Circle<T>& circle) { return circle.contains(*this); }

// ====== [TRANSLATION] ====== //

template <typename T>
void Point<T>::translate(T deltaX, T deltaY) { x += deltaX; y += deltaY; }
template <typename T>
void Point<T>::translate(const Point<T>& other) { *this += other; }
template <typename T>
Point<T> Point<T>::translated(T deltaX, T deltaY) const { return Point<T>(x + deltaX, y + deltaY); }
template <typename T>
Point<T> Point<T>::translated(const Point<T>& other) const { return *this + other; }

template <typename T>
Point<T> Point<T>::midpoint(const Point<T>& other) const { return Point<T>((x + other.x) / 2, (y + other.y) / 2); }

template <typename T>
void Point<T>::interpolate(const Point<T>& other, float t) { x = x + (other.x - x) * t; y = y + (other.y - y) * t; }
template <typename T>
Point<T> Point<T>::interpolated(const Point<T>& other, float t) const { return Point<T>(x + (other.x - x) * t, y + (other.y - y) * t); }

// ====== [SCALE] ====== //

template <typename T>
void Point<T>::scaleFromOrigin(float scale) { *this *= scale; }
template <typename T>
Point<T> Point<T>::scaledFromOrigin(float scale) const { return *this * scale; }

template <typename T>
void Point<T>::scaleFromPoint(const Point<T>& other, float scale) {
    x = (x - other.x) * scale + other.x;
    y = (y - other.y) * scale + other.y;
}
template <typename T>
Point<T> Point<T>::scaledFromPoint(const Point<T>& other, float scale) const {
    return Point<T>((x - other.x) * scale + other.x, (y - other.y) * scale + other.y);
}

// ====== [ROTATION] ====== //

template <typename T>
float Point<T>::angle() const {
    float a = atan2f(y, x);
    return a >= 0 ? a : a + 2 * PI;
}

template <typename T>
float Point<T>::angleTo(const Point<T>& other) const { return atan2f(other.y - y, other.x - x); }

template <typename T>
void Point<T>::rotateAroundOrigin(float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    T nx = x * cosA - y * sinA;
    T ny = x * sinA + y * cosA;
    x = nx; y = ny;
}

template <typename T>
Point<T> Point<T>::rotatedAroundOrigin(float angle) const {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    return Point<T>(x * cosA - y * sinA, x * sinA + y * cosA);
}

template <typename T>
void Point<T>::rotateAroundPoint(const Point<T>& other, float angle) {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    float dx = x - other.x;
    float dy = y - other.y;
    x = dx * cosA - dy * sinA + other.x;
    y = dx * sinA + dy * cosA + other.y;
}

template <typename T>
Point<T> Point<T>::rotatedAroundPoint(const Point<T>& other, float angle) const {
    float cosA = cosf(angle);
    float sinA = sinf(angle);
    float dx = x - other.x;
    float dy = y - other.y;
    return Point<T>(dx * cosA - dy * sinA + other.x, dx * sinA + dy * cosA + other.y);
}

// ====== [REFLECTION] ====== //

template <typename T>
void Point<T>::reflectAcrossHorizontal(T horizontal) { y = 2 * horizontal - y; }
template <typename T>
Point<T> Point<T>::reflectedAcrossHorizontal(T horizontal) const { return Point<T>(x, 2 * horizontal - y); }

template <typename T>
void Point<T>::reflectAcrossVertical(T vertical) { x = 2 * vertical - x; }
template <typename T>
Point<T> Point<T>::reflectedAcrossVertical(T vertical) const { return Point<T>(2 * vertical - x, y); }

template <typename T>
void Point<T>::reflectAcrossLine(Line<T> line) {
    float m = line.slope();
    float b = line.intercept();
    float d = (x + (y - b) * m) / (1 + m * m);
    x = 2 * d - x;
    y = 2 * d * m - y + 2 * b;
}

template <typename T>
Point<T> Point<T>::reflectedAcrossLine(Line<T> line) const {
    float m = line.slope();
    float b = line.intercept();
    float d = (x + (y - b) * m) / (1 + m * m);
    return Point<T>(2 * d - x, 2 * d * m - y + 2 * b);
}

// ====== [CONVERSION] ====== //

template <typename T>
Point<int> Point<T>::toInt() const { return Point<int>(static_cast<int>(x), static_cast<int>(y)); }
template <typename T>
Point<float> Point<T>::toFloat() const { return Point<float>(static_cast<float>(x), static_cast<float>(y)); }

template struct Point<int>;
template struct Point<float>;