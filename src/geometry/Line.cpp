#include "Line.h"

#include <algorithm>
#include <cmath>
#include <optional>
#include <vector>
#include <limits>

#include "Rect.h"
#include "Circle.h"

template<typename T>
inline Line<T>::Line(T x1, T y1, T x2, T y2) : p1(x1, y1), p2(x2, y2) {}

template<typename T>
Line<T>::Line(const Point<T>& p1, const Point<T>& p2) : p1(p1), p2(p2) {}

template<typename T>
Line<float> Line<T>::radialLine(float angle, float startRadius, float endRadius) {
    Point<float> unitVector = Point<float>::getPointOnUnitCircle(angle);
    return Line<float>(unitVector * startRadius, unitVector * endRadius);
}

template<typename T>
Line<float> Line<T>::radialLine(float angle, float startRadiusX, float startRadiusY, float endRadiusX, float endRadiusY) {
    return Line<float>(Point<float>::getPointOnCircumference(startRadiusX, startRadiusY, angle),
                        Point<float>::getPointOnCircumference(endRadiusX, endRadiusY, angle));
}

template<typename T>
bool Line<T>::operator==(const Line<T>& other) const { return p1 == other.p1 && p2 == other.p2; }

template<typename T>
bool Line<T>::operator!=(const Line<T>& other) const { return !(*this == other); }

template<typename T>
Line<T> Line<T>::operator-() const { return Line<T>(-p1, -p2); }

template<typename T>
Line<T> Line<T>::reversed() const { return Line<T>(p2, p1); }

template<typename T>
bool Line<T>::isHorizontal() const { return p1.y == p2.y; }

template<typename T>
bool Line<T>::isVertical() const { return p1.x == p2.x; }

template<typename T>
float Line<T>::length() const { return p1.distanceTo(p2); }

template<typename T>
float Line<T>::normal() const { return p1.angleTo(p2) - 3.14159265f/2; }

template<typename T>
float Line<T>::slope() const { return (p2.x == p1.x) ? std::numeric_limits<float>::infinity() : (p2.y - p1.y) / (p2.x - p1.x); }

template<typename T>
float Line<T>::intercept() const { return (p2.x == p1.x) ? std::numeric_limits<float>::infinity() : p1.y - slope() * p1.x; }

template<typename T>
Point<T> Line<T>::midpoint() const { return p1.midpoint(p2); }

template<typename T>
float Line<T>::angle() const { return p1.angleTo(p2); }

template<typename T>
bool Line<T>::intersects(const Line<T>& other) const {
    float denom = (other.p2.y - other.p1.y)*(p2.x - p1.x) - (other.p2.x - other.p1.x)*(p2.y - p1.y);
    if (denom == 0.0f) return false;
    float uA = ((other.p2.x - other.p1.x)*(p1.y - other.p1.y) - (other.p2.y - other.p1.y)*(p1.x - other.p1.x)) / denom;
    float uB = ((p2.x - p1.x)*(p1.y - other.p1.y) - (p2.y - p1.y)*(p1.x - other.p1.x)) / denom;
    return (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1);
}

template<typename T>
std::optional<Point<T>> Line<T>::intersection(const Line<T>& other) const {
    float denom = (other.p2.y - other.p1.y)*(p2.x - p1.x) - (other.p2.x - other.p1.x)*(p2.y - p1.y);
    if (denom == 0.0f) return std::nullopt;
    float uA = ((other.p2.x - other.p1.x)*(p1.y - other.p1.y) - (other.p2.y - other.p1.y)*(p1.x - other.p1.x)) / denom;
    float uB = ((p2.x - p1.x)*(p1.y - other.p1.y) - (p2.y - p1.y)*(p1.x - other.p1.x)) / denom;
    if (uA >= 0 && uA <= 1 && uB >= 0 && uB <= 1) return Point<T>(p1.x + uA*(p2.x - p1.x), p1.y + uA*(p2.y - p1.y));
    return std::nullopt;
}

template<typename T>
std::optional<Point<T>> Line<T>::slopeIntersection(const Line<T>& other) const {
    float denom = (other.p2.y - other.p1.y)*(p2.x - p1.x) - (other.p2.x - other.p1.x)*(p2.y - p1.y);
    if (denom == 0.0f) return std::nullopt;
    float uA = ((other.p2.x - other.p1.x)*(p1.y - other.p1.y) - (other.p2.y - other.p1.y)*(p1.x - other.p1.x)) / denom;
    return Point<T>(p1.x + uA*(p2.x - p1.x), p1.y + uA*(p2.y - p1.y));
}

template<typename T>
bool Line<T>::intersects(const Rect<T>& rect) const {
    const auto& edges = rect.getEdges();

    int count = 0;

    if (rect.contains(p1)) count++;
    if (rect.contains(p2)) count++;
    if (count == 2) return true;

    for (const auto& edge : edges) if (intersects(edge) && (++count == 2)) return true;

    return false;
}

template<typename T>
std::optional<Line<T>> Line<T>::intersection(const Rect<T>& rect) const {
    const auto& edges = rect.getEdges();

    std::vector<Point<T>> points;

    if (rect.contains(p1)) points.push_back(p1);
    if (rect.contains(p2)) points.push_back(p2);
    if (points.size() == 2) return Line<T>(points[0], points[1]); // Fully contained line

    for (const auto& edge : edges) {
        auto point = intersection(edge);
        if (point) {
            points.push_back(*point);
            if (points.size() == 2) return Line<T>(points[0], points[1]);
        }
    }

    return std::nullopt;
}

template<typename T>
std::vector<Line<T>> Line<T>::intersection(const std::vector<Rect<T>>& rectangles) const {
    std::vector<Line<T>> lines;

    for (const auto& rectangle : rectangles) {
        auto line = intersection(rectangle);
        //if (line) lines.push_back(line);
    }

    return lines;
}

template<typename T>
bool Line<T>::isContainedBy(const Rect<T>& rectangle) { return true; //return rectangle.contains(*this);
    }

template<typename T>
bool Line<T>::isContainedBy(const Circle<T>& circle) { return circle.contains(*this); }

template<typename T>
Line<T> Line<T>::operator+(const Point<T>& translate) const { return Line<T>(p1 + translate, p2 + translate); }

template<typename T>
Line<T>& Line<T>::operator+=(const Point<T>& translate) { p1 += translate; p2 += translate; return *this; }

template<typename T>
Line<T> Line<T>::operator-(const Point<T>& translate) const { return Line<T>(p1 - translate, p2 - translate); }

template<typename T>
Line<T>& Line<T>::operator-=(const Point<T>& translate) { p1 -= translate; p2 -= translate; return *this; }

template<typename T>
void Line<T>::translate(T deltaX, T deltaY) { p1.translate(deltaX, deltaY); p2.translate(deltaX, deltaY); }

template<typename T>
void Line<T>::translate(const Point<T>& delta) { p1.translate(delta); p2.translate(delta); }

template<typename T>
Line<T> Line<T>::translated(T deltaX, T deltaY) const { return Line<T>(p1.translated(deltaX, deltaY), p2.translated(deltaX, deltaY)); }

template<typename T>
Line<T> Line<T>::translated(const Point<T>& delta) const { return Line<T>(p1.translated(delta), p2.translated(delta)); }

template<typename T>
void Line<T>::interpolate(const Line<T>& other, float t) { p1.interpolate(other.p1, t); p2.interpolate(other.p2, t); }

template<typename T>
Line<T> Line<T>::interpolated(const Line<T>& other, float t) const { return Line<T>(p1.interpolated(other.p1, t), p2.interpolated(other.p2, t)); }

template<typename T>
Line<T> Line<T>::operator*(float scalar) const { return Line<T>(p1 * scalar, p2 * scalar); }

template<typename T>
Line<T>& Line<T>::operator*=(const Point<T>& scale) { p1 *= scale; p2 *= scale; return *this; }

template<typename T>
Line<T> Line<T>::operator/(float scalar) const { return Line<T>(p1 / scalar, p2 / scalar); }

template<typename T>
Line<T>& Line<T>::operator/=(const Point<T>& scale) { p1 /= scale; p2 /= scale; return *this; }

template<typename T>
void Line<T>::scaleFromOrigin(float scale) { p1.scaleFromOrigin(scale); p2.scaleFromOrigin(scale); }

template<typename T>
Line<T> Line<T>::scaledFromOrigin(float scale) { return Line<T>(p1.scaledFromOrigin(scale), p2.scaledFromOrigin(scale)); }

template<typename T>
void Line<T>::scaleFromPoint(const Point<T>& other, float scale) { p1.scaleFromPoint(other, scale); p2.scaleFromPoint(other, scale); }

template<typename T>
Line<T> Line<T>::scaledFromPoint(const Point<T>& other, float scale) { return Line<T>(p1.scaledFromPoint(other, scale), p2.scaledFromPoint(other, scale)); }

template<typename T>
void Line<T>::rotateAroundOrigin(float angle) { p1.rotateAroundOrigin(angle); p2.rotateAroundOrigin(angle); }

template<typename T>
Line<T> Line<T>::rotatedAroundOrigin(float angle) { return Line<T>(p1.rotatedAroundOrigin(angle), p2.rotatedAroundOrigin(angle)); }

template<typename T>
void Line<T>::rotateAroundPoint(const Point<T>& pivot, float angle) { p1.rotateAroundPoint(pivot, angle); p2.rotateAroundPoint(pivot, angle); }

template<typename T>
Line<T> Line<T>::rotatedAroundPoint(const Point<T>& pivot, float angle) { return Line<T>(p1.rotatedAroundPoint(pivot, angle), p2.rotatedAroundPoint(pivot, angle)); }

template<typename T>
void Line<T>::reflectAcrossHorizontal(T horizontal) { p1.reflectAcrossHorizontal(horizontal); p2.reflectAcrossHorizontal(horizontal); }

template<typename T>
Line<T> Line<T>::reflectedAcrossHorizontal(T horizontal) { return Line<T>(p1.reflectedAcrossHorizontal(horizontal), p2.reflectedAcrossHorizontal(horizontal)); }

template<typename T>
void Line<T>::reflectAcrossVertical(T vertical) { p1.reflectAcrossVertical(vertical); p2.reflectAcrossVertical(vertical); }

template<typename T>
Line<T> Line<T>::reflectedAcrossVertical(T vertical) { return Line<T>(p1.reflectedAcrossVertical(vertical), p2.reflectedAcrossVertical(vertical)); }

template<typename T>
void Line<T>::reflectAcrossLine(Line<T> line) { p1.reflectAcrossLine(line); p2.reflectAcrossLine(line); }

template<typename T>
Line<T> Line<T>::reflectedAcrossLine(Line<T> line) { return Line<T>(p1.reflectedAcrossLine(line), p2.reflectedAcrossLine(line)); }

template<typename T>
Line<int> Line<T>::toInt() const { return Line<int>(p1.toInt(), p2.toInt()); }

template<typename T>
Line<float> Line<T>::toFloat() const { return Line<float>(p1.toFloat(), p2.toFloat()); }

template struct Line<int>;
template struct Line<float>;