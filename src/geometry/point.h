#pragma once

#include "alignment.h"

template <typename T>
struct Line;

template <typename T>
struct Rect;

template <typename T>
struct Circle;

template <typename T>
struct Point {
    T x, y;

    Point();
    Point(T x, T y);

    /// @brief Returns a unit vector pointing in the specified angle.
    /// @param angle Angle in radians.
    /// @return A Point representing a unit vector with the given direction
    static Point<float> getPointOnUnitCircle(float angle);

    static Point<float> getPointOnCircumference(float radius, float angle);
    static Point<float> getPointOnCircumference(float radiusX, float radiusY, float angle);

    static Point<T> getAnchored(T x, T y, T width, T height, Anchor anchor);
    static Point<T> getAnchored(const Point<T>& point, T width, T height, Anchor anchor);
    static Point<T> getAnchored(const Rect<T>& rectangle, Anchor anchor);

    inline bool operator==(const Point<T>& other) const;
    inline bool operator!=(const Point<T>& other) const;

    inline Point<T> operator-() const;

    inline Point<T>& operator=(const Point<T>& other);
    inline Point<T>& operator+=(const Point<T>& other);
    inline Point<T>& operator-=(const Point<T>& other);
    inline Point<T>& operator*=(const Point<T>& other);
    inline Point<T>& operator/=(const Point<T>& other);

    inline Point<T>& operator*=(float scalar);
    inline Point<T>& operator/=(float scalar);

    inline Point<T> operator-(const Point<T>& other) const;
    inline Point<T> operator+(const Point<T>& other) const;
    inline Point<T> operator*(const Point<T>& other) const;
    inline Point<T> operator/(const Point<T>& other) const;

    inline Point<T> operator*(float scalar) const;
    inline Point<T> operator/(float scalar) const;

    inline Point<T> operator^(float exponent) const;

    float magnitude() const;

    Point<float> normalized() const;

    float distance() const;
    float distanceTo(const Point<T>& other) const;
    float dot(const Point<T>& other) const;
    float cross(const Point<T>& other) const;

    // ====== [INTERSECTIONS] ====== //

    bool isContainedBy(const Rect<T>& rectangle);
    bool isContainedBy(const Circle<T>& circle);

    // ====== [TRANSLATION] ====== //
    
    void translate(T deltaX, T deltaY);
    void translate(const Point<T>& other);
    Point<T> translated(T deltaX, T deltaY) const;
    Point<T> translated(const Point<T>& other) const;

    Point<T> midpoint(const Point<T>& other) const;

    void interpolate(const Point<T>& other, float t);
    Point<T> interpolated(const Point<T>& other, float t) const;

    // ====== [SCALE] ====== //

    void scaleFromOrigin(float scale);
    Point<T> scaledFromOrigin(float scale) const;

    void scaleFromPoint(const Point<T>& other, float scale);
    Point<T> scaledFromPoint(const Point<T>& other, float scale) const;

    // ====== [ROTATION] ====== //

    float angle() const;
    float angleTo(const Point<T>& other) const;

    void rotateAroundOrigin(float angle);
    Point<T> rotatedAroundOrigin(float angle) const;

    void rotateAroundPoint(const Point<T>& other, float angle);
    Point<T> rotatedAroundPoint(const Point<T>& other, float angle) const;

    // ====== [REFLECTION] ====== //

    void reflectAcrossHorizontal(T horizontal);
    Point<T> reflectedAcrossHorizontal(T horizontal) const;

    void reflectAcrossVertical(T vertical);
    Point<T> reflectedAcrossVertical(T vertical) const;

    void reflectAcrossLine(Line<T> line);
    Point<T> reflectedAcrossLine(Line<T> line) const;

    // ====== [CONVERSION] ====== //

    Point<int> toInt() const;
    Point<float> toFloat() const;
};

extern template struct Point<int>;
extern template struct Point<float>;