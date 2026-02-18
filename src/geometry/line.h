#pragma once

#include "point.h"

#include <optional>
#include <vector>

template <typename T>
class Rect;

template <typename T>
class Circle;

template <typename T>
struct Line {
    Point<T> p1, p2;

    Line(T x1, T y1, T x2, T y2);
    Line(const Point<T>& p1, const Point<T>& p2);

    static Line<float> radialLine(float angle, float startRadius, float endRadius);

    static Line<float> radialLine(float angle, float startRadiusX, float startRadiusY, float endRadiusX, float endRadiusY);

    bool operator==(const Line<T>& other) const;
    bool operator!=(const Line<T>& other) const;
    Line<T> operator-() const;

    Line<T> reversed() const;

    bool isHorizontal() const;
    bool isVertical() const;

    float length() const;
    float normal() const;

    float slope() const;
    float intercept() const;

    Point<T> midpoint() const;

    float angle() const;

    // ====== [INTERSECTIONS] ====== //

    bool intersects(const Line<T>& other) const;

    std::optional<Point<T>> intersection(const Line<T>& other) const;

    bool slopesIntersect(const Line<T>& other) const { return slope() != other.slope(); }

    std::optional<Point<T>> slopeIntersection(const Line<T>& other) const;

    bool intersects(const Rect<T>& rect) const;

    std::optional<Line<T>> intersection(const Rect<T>& rect) const;

    bool intersects(const std::vector<Rect<T>>& rectangles) const { for (const auto& rectangle : rectangles) if (intersects(rectangle)) return true; return false; }

    std::vector<Line<T>> intersection(const std::vector<Rect<T>>& rectangles) const;
    bool isContainedBy(const Rect<T>& rectangle);
    bool isContainedBy(const Circle<T>& circle);

    // ====== [TRANSLATION] ====== //

    Line<T> operator+(const Point<T>& translate) const;
    Line<T>& operator+=(const Point<T>& translate);
    Line<T> operator-(const Point<T>& translate) const;
    Line<T>& operator-=(const Point<T>& translate);

    void translate(T deltaX, T deltaY);
    void translate(const Point<T>& delta);
    Line<T> translated(T deltaX, T deltaY) const;
    Line<T> translated(const Point<T>& delta) const;

    void interpolate(const Line<T>& other, float t);
    Line<T> interpolated(const Line<T>& other, float t) const;

    // ====== [SCALING] ====== //

    Line<T> operator*(float scalar) const;
    Line<T>& operator*=(const Point<T>& scale);
    Line<T> operator/(float scalar) const;
    Line<T>& operator/=(const Point<T>& scale);

    void scaleFromOrigin(float scale);
    Line<T> scaledFromOrigin(float scale);

    void scaleFromPoint(const Point<T>& other, float scale);
    Line<T> scaledFromPoint(const Point<T>& other, float scale);

    // ====== [ROTATION] ====== //

    void rotateAroundOrigin(float angle);
    Line<T> rotatedAroundOrigin(float angle);

    void rotateAroundPoint(const Point<T>& pivot, float angle);
    Line<T> rotatedAroundPoint(const Point<T>& pivot, float angle);

    // ====== [REFLECTION] ====== //

    void reflectAcrossHorizontal(T horizontal);
    Line<T> reflectedAcrossHorizontal(T horizontal);

    void reflectAcrossVertical(T vertical);
    Line<T> reflectedAcrossVertical(T vertical);

    void reflectAcrossLine(Line<T> line);
    Line<T> reflectedAcrossLine(Line<T> line);

    // ====== [CONVERSION] ====== //

    Line<int> toInt() const;
    Line<float> toFloat() const;
};

extern template struct Line<int>;
extern template struct Line<float>;