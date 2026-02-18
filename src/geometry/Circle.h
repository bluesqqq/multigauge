#pragma once

#include "Point.h"
#include "Rect.h"

template <typename T>
struct Circle {
    Point<T> position;
    T radius;

    Circle(T x, T y, T radius);
    Circle(const Point<T>& pos, T radius);

    T area() const;
    T circumference() const;

    bool operator==(const Rect<T>& other) const;
    bool operator!=(const Rect<T>& other) const;

    // ====== [INTERSECTIONS] ====== //

    // Point

    bool contains(const Point<T>& point) const;

    // Line

    bool intersects(const Line<T>& line) const;
    std::optional<Line<T>> intersection(const Line<T>& line) const;
    bool contains(const Line<T>& line) const;

    // Rect 

    bool intersects(const Rect<T>& other) const;
    std::optional<Path<T>> intersection(const Rect<T>& other) const;
    bool contains(const Rect<T>& other) const;

    // Circle

    bool intersects(const Circle<T>& other) const;
    std::optional<Path<T>> intersection(const Circle<T>& other) const;
    bool contains(const Circle<T>& other) const;

};