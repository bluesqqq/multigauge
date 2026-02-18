#pragma once

#include "Rect.h"

template <typename T>
class Path {
    public:
        Path();
        Path(const Rect<T>& rectangle);

        Rect<T> getBounds() const;

        bool contains(Point<T>);
        bool intersects(Line<T> line);
        bool intersects(Rect<T> rectangle);

        Point<float> getPointAlongPath(float distanceFromStart) const;

        // ====== [PATH BUILDING] ====== //

        void clear();
        void startNewSubPath(Point<T> start);
        void closeSubPath();
        void lineTo(Point<T> end);
        void addRect(Rect<T> rectangle);

        // ====== [TRANSLATION] ====== //

        void translate(T deltaX, T deltaY);
        void translate(const Point<T>& other);
        Path<T> translated(T deltaX, T deltaY);
        Path<T> translated(const Point<T>& other);

        // ====== [SCALE] ====== //

        void scaleFromOrigin(float scale);
        Path<T> scaledFromOrigin(float scale);

        void scaleFromPoint(const Point<T>& other, float scale);
        Path<T> scaledFromPoint(const Point<T>& other, float scale);

        // ====== [ROTATION] ====== //

        float angle() const;

        void rotateAroundOrigin(float angle);
        Path<T> rotatedAroundOrigin(float angle);

        void rotateAroundPoint(const Point<T>& other, float angle);
        Path<T> rotatedAroundPoint(const Point<T>& other, float angle);

        // ====== [REFLECTION] ====== //

        void reflectAcrossHorizontal(T horizontal);
        Path<T> reflectedAcrossHorizontal(T horizontal);

        void reflectAcrossVertical(T vertical);
        Path<T> reflectedAcrossVertical(T vertical);

        void reflectAcrossLine(Line<T> line);
        Path<T> reflectedAcrossLine(Line<T> line);

        // ====== [CONVERSION] ====== //

        Path<int> toInt() const;
        Path<float> toFloat() const;
};

template <typename T>
inline Path<T>::Path(const Rect<T> &rectangle) {
    addRect(rectangle);
}
