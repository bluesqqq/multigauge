#pragma once

#include <optional>

#include "point.h"
#include "alignment.h"
#include "Line.h"
#include "path.h"
#include "utils.h"

#include <vector>

template <typename T>
struct Rect {
    T x, y;
    T width, height;

    Rect(T x, T y, T width, T height);
    Rect(const Point<T>& pos, T w, T h);

    static Rect<T> fromPoints(Point<T> p1, Point<T> p2);

    Point<T> position() const;
    T area() const;
    T perimeter() const;
    bool isEmpty() const;
    
    bool operator==(const Rect<T>& other) const;
    bool operator!=(const Rect<T>& other) const;
    
    // ====== [INTERSECTIONS] ====== //

    // Point

    bool contains(const Point<T>& point) const;

    // Line

    bool intersects(const Line<T>& line) const;
    std::optional<Line<T>> intersection(const Line<T>& line) const;
    bool contains(const Line<T>& line);

    // Rect 

    bool intersects(const Rect<T>& other) const;
    std::optional<Rect<T>> intersection(const Rect<T>& other) const;
    bool contains(const Rect<T>& other) const;

    // Circle

    bool intersects(const Circle<T>& circle) const;
    std::optional<Path<T>> intersection(const Circle<T>& circle) const;
    bool contains(const Circle<T>& circle) const;

    // ====== [POSITIONING] ====== //

    void setTop(T top);
    void setBottom(T bottom);
    void setLeft(T left);
    void setRight(T right);
    void setCenterX(T centerX);
    void setCenterY(T centerY);

    T getTop()     const;
    T getBottom()  const;
    T getLeft()    const;
    T getRight()   const;
    T getCenterX() const;
    T getCenterY() const;

    int getBottomPixel() const;
    int getRightPixel() const;

    Point<T> getTopLeft()     const;
    Point<T> getBottomLeft()  const;
    Point<T> getTopRight()    const;
    Point<T> getBottomRight() const;
    Point<T> getCenter()      const;

    Point<int> getBottomLeftPixel()  const;
    Point<int> getBottomRightPixel() const;
    Point<int> getTopRightPixel()    const;

    Line<T> getTopEdge()    const;
    Line<T> getBottomEdge() const;
    Line<T> getLeftEdge()   const;
    Line<T> getRightEdge()  const;
    std::vector<Line<T>> getEdges() const;

    // ====== [SHAPING] ====== //

    void trimTop(T amount);
    void trimBottom(T amount);
    void trimLeft(T amount);
    void trimRight(T amount);

    Rect<T> trimmedTop(T amount) const;
    Rect<T> trimmedBottom(T amount) const;
    Rect<T> trimmedLeft(T amount) const;
    Rect<T> trimmedRight(T amount) const;

    Rect<T> removeFromTop(T amount);
    Rect<T> removeFromBottom(T amount);
    Rect<T> removeFromLeft(T amount);
    Rect<T> removeFromRight(T amount);

    // ====== [TRANSLATION] ====== //

    Rect<T> operator+(Point<T> delta) const;
    Rect<T>& operator+=(Point<T> delta);
    Rect<T> operator-(Point<T> delta) const;
    Rect<T>& operator-=(Point<T> delta);

    void translate(T deltaX, T deltaY);
    void translate(Point<T> delta);
    Rect<T> translated(T deltaX, T deltaY) const;
    Rect<T> translated(Point<T> delta) const;

    void interpolate(const Rect<T>& other, float t);
    Rect<T> interpolated(const Rect<T>& other, float t) const;

    // ====== [SCALING] ====== //

    Rect<T>  operator*(float scale) const;
    Rect<T>& operator*=(float scale);
    Rect<T>  operator/(float scale) const;
    Rect<T>& operator/=(float scale);

    void scaleFromOrigin(float scale);
    Rect<T> scaledFromOrigin(float scale) const;

    void scaleFromPoint(const Point<T>& other, float scale);
    Rect<T> scaledFromPoint(const Point<T>& other, float scale) const;

    void reduce(T deltaX, T deltaY);
    void reduce(T delta);

    Rect<T> reduced(T deltaX, T deltaY) const;
    Rect<T> reduced(T delta) const;

    void expand(T deltaX, T deltaY);
    void expand(T delta);

    Rect<T> expanded(T deltaX, T deltaY) const;
    Rect<T> expanded(T delta) const;

    // ====== [ROTATION] ====== //

    void rotateAroundOrigin(float angle);
    Rect<T> rotatedAroundOrigin(float angle) const;

    void rotateAroundPoint(const Point<T>& other, float angle);
    Rect<T> rotatedAroundPoint(const Point<T>& other, float angle) const;

    // ====== [REFLECTION] ====== //

    void reflectAcrossHorizontal(T horizontal);
    Rect<T> reflectedAcrossHorizontal(T horizontal) const;

    void reflectAcrossVertical(T vertical);
    Rect<T> reflectedAcrossVertical(T vertical) const;

    // ====== [CONVERSION] ====== //

    Rect<float> toFloat() const;
    Rect<int> toInt() const;
    Path<T> asPath() const;
};

extern template struct Rect<int>;
extern template struct Rect<float>;