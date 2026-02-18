#pragma once

#include "graphics/gauge/Element.h"
#include "values/Value.h"
#include <vector>
#include <cmath>

class GForceElement : public Element {
private:
    int numberOfPoints = 24;

    std::vector<float> maxGForce;

    // Current g vector in normalized-ish units (your old code used Point in [-1..1] range)
    Point<float> gforce = Point<float>(0.0f, 0.0f);

public:
    GForceElement(Element* parent, int points = 24) : Element(parent), numberOfPoints(points) {
        if (numberOfPoints < 3) numberOfPoints = 3;
        maxGForce.resize((size_t)numberOfPoints, 0.3f);
    }

    GForceElement(Element* parent, const rapidjson::Value::ConstObject json) : Element(parent, json) { }

    void update(int deltaTime) override {
        (void)deltaTime;

        // Same motion as your old code, just explicit floats.
        const float t1 = millis() * 0.001f;
        const float t2 = millis() * 0.0033f;

        // If you already have Point::unitVector(angle), use it.
        // Otherwise: unit = (cos(angle), sin(angle)).
        Point<float> unit(std::cos(t1), std::sin(t1));
        gforce = unit * std::sin(t2);

        // angle in [0..2PI)
        float angle = std::atan2(gforce.y, gforce.x);
        if (angle < 0) angle += 2.0f * PI;

        const float mag = std::sqrt(gforce.x * gforce.x + gforce.y * gforce.y);

        int index = int(angle / (2.0f * PI) * numberOfPoints);
        if (index < 0) index = 0;
        if (index >= numberOfPoints) index = numberOfPoints - 1;

        if (mag > maxGForce[(size_t)index]) {
            maxGForce[(size_t)index] = mag;
        }
    }

    void draw(Graphics& g) const override {
        const auto b = getBounds().toInt();

        const int cx = b.getCenterX();
        const int cy = b.getCenterY();
        const int r  = std::min(b.width, b.height) / 2;

        // Guard
        if (r <= 0 || numberOfPoints < 3) return;

        // Rings
        g.setStroke(rgb(255, 255, 255));
        g.strokeCircle(cx, cy, r);
        g.strokeCircle(cx, cy, r / 2);

        // Build and draw polygon outline from maxGForce bins
        const float angleSpacing = (2.0f * PI) / float(numberOfPoints);

        int firstX = 0, firstY = 0;
        int prevX = 0, prevY = 0;

        for (int i = 0; i < numberOfPoints; ++i) {
            const float angle = i * angleSpacing;
            const float mag   = maxGForce[(size_t)i];

            const float px = cx + std::cos(angle) * mag * r;
            const float py = cy + std::sin(angle) * mag * r;

            const int ix = (int)std::lround(px);
            const int iy = (int)std::lround(py);

            if (i == 0) {
                firstX = ix; firstY = iy;
                prevX  = ix; prevY  = iy;
            } else {
                g.setStroke(rgb(255, 255, 255));
                g.strokeLine(prevX, prevY, ix, iy);
                prevX = ix; prevY = iy;
            }
        }

        // Close polygon
        g.setStroke(rgb(255, 0, 0));
        g.strokeLine(prevX, prevY, firstX, firstY);

        // Current g dot
        const int dotX = (int)std::lround(cx + gforce.x * r);
        const int dotY = (int)std::lround(cy + gforce.y * r);

        g.setFill(rgb(255, 255, 0));
        g.fillCircle(dotX, dotY, 8);
    }
};

REGISTER_ELEMENT_TYPE("g-force", GForceElement);