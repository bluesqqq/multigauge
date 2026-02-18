#pragma once

#include "graphics/gauge/Element.h"
#include "graphics/colors/StaticColor.h"
#include <algorithm>
#include <cmath>

class ThrottleElement : public Element {
private:
    // Tune these as proportions of size (not pixels)
    float bodySpacingRatio   = 0.08f;  // how far bore is inset from body
    float sideThicknessRatio = 0.08f;  // the "ears" thickness

    Color* bodyFill        = nullptr;
    Color* bodyStroke      = nullptr;
    Color* plateFill       = nullptr;
    Color* plateStroke     = nullptr;
    Color* boltFill        = nullptr;
    Color* boltStroke      = nullptr;

    // Your source used throttlePosition.getInterpolationValue()
    // If you still have a Value somewhere, store a reference; otherwise stub a float.
    Value& throttlePosition;

public:
    ThrottleElement(Element* parent, const rapidjson::Value::ConstObject json) : Element(parent, json) { }

    void draw(Graphics& g) const override {
        const auto b = getBounds().toInt();
        if (b.width <= 0 || b.height <= 0) return;

        const int cx = b.getCenterX();
        const int cy = b.getCenterY();

        // Use the smaller dimension so we stay inside bounds.
        const int minDim = std::min(b.width, b.height);

        // Outer body square size, kept inside bounds
        const float bodySize = float(minDim);

        // Derive "throttle radius" from body size and spacing ratio
        const float bodySpacing   = bodySize * bodySpacingRatio;
        const float throttleR     = (bodySize * 0.5f) - bodySpacing;
        const float sideThickness = bodySize * sideThicknessRatio;

        if (throttleR <= 1.0f) return;

        // Interpolation for plate (0..1)
        float t = 0.0f;
        if (throttlePosition) t = throttlePosition.getInterpolationValue();
        if (t < 0.0f) t = 0.0f;
        if (t > 1.0f) t = 1.0f;

        // -------- Body (two Rects like your original) --------
        // In original:
        // 1) wide rect: (bodySize + 2*sideThickness) x (bodySize - 2*sideThickness)
        // 2) main square: bodySize x bodySize
        //
        // We keep both inside bounds by scaling them to bodySize=minDim.

        const float wideW = bodySize; // already maxed by minDim
        const float wideH = std::max(1.0f, bodySize - 2.0f * sideThickness);

        // Draw "wide" rect as horizontal ears: extend in X by sideThickness, shrink in Y
        // But we must remain inside bounds, so instead we simulate ears by drawing
        // a rect that fits, then a second square on top (same as original look).
        g.setFill(bodyFill->getColor());
        g.fillRect(
            int(std::lround(cx - wideW * 0.5f)),
            int(std::lround(cy - wideH * 0.5f)),
            int(std::lround(wideW)),
            int(std::lround(wideH))
        );

        g.setStroke(bodyStroke->getColor());
        g.strokeRect(Rect<int>(
            int(std::lround(cx - wideW * 0.5f)),
            int(std::lround(cy - wideH * 0.5f)),
            int(std::lround(wideW)),
            int(std::lround(wideH))
        ));

        // Main square
        g.setFill(bodyFill->getColor());
        g.fillRect(
            int(std::lround(cx - bodySize * 0.5f)),
            int(std::lround(cy - bodySize * 0.5f)),
            int(std::lround(bodySize)),
            int(std::lround(bodySize))
        );

        g.setStroke(bodyStroke->getColor());
        g.strokeRect(Rect<int>(
            int(std::lround(cx - bodySize * 0.5f)),
            int(std::lround(cy - bodySize * 0.5f)),
            int(std::lround(bodySize)),
            int(std::lround(bodySize))
        ));

        // -------- Bore (circle) --------
        g.setFill(bodyFill->getColor());
        g.fillCircle(cx, cy, int(std::lround(throttleR)));

        g.setStroke(bodyStroke->getColor());
        g.strokeCircle(cx, cy, int(std::lround(throttleR)));

        // -------- Bolts (4 small circles) --------
        const float boltR = std::max(1.0f, bodySpacing * 0.5f);
        const int   bxOff = int(std::lround(throttleR));
        const int   byOff = int(std::lround(throttleR));

        g.setFill(boltFill->getColor());
        g.setStroke(boltStroke->getColor());

        auto bolt = [&](int x, int y) {
            g.fillCircle(x, y, int(std::lround(boltR)));
            g.strokeCircle(x, y, int(std::lround(boltR)));
        };

        bolt(cx - bxOff, cy - byOff);
        bolt(cx - bxOff, cy + byOff);
        bolt(cx + bxOff, cy - byOff);
        bolt(cx + bxOff, cy + byOff);

        // -------- Throttle plate (ellipse) --------
        // Original logic: rx = (throttleR - 1) * t, ry = (throttleR - 1)
        const float plateRxF = std::max(0.0f, (throttleR - 1.0f) * t);
        const float plateRyF = std::max(0.0f, (throttleR - 1.0f));

        const int plateRx = (int)std::lround(plateRxF);
        const int plateRy = (int)std::lround(plateRyF);

        // If rx or ry collapse, skip fill (or draw a line if you want)
        if (plateRx > 0 && plateRy > 0)
        {
            // Bounding box for ellipse: left/top + width/height
            // width  = 2*rx, height = 2*ry
            const int plateLeft = cx - plateRx;
            const int plateTop  = cy - plateRy;
            const int plateW    = plateRx * 2;
            const int plateH    = plateRy * 2;

            const Rect<int> plateArea(plateLeft, plateTop, plateW, plateH);

            g.setFill(plateFill->getColor());
            //g.fillEllipse(plateArea);

            g.setStroke(plateStroke->getColor());
            //g.strokeEllipse(plateArea);
        }


        // -------- Throttle shaft (thin vertical rect) --------
        const float shaftW = std::max(1.0f, bodySize * 0.03f);
        const float shaftH = std::max(1.0f, (throttleR - 1.0f) * 2.0f);

        g.setFill(bodyFill->getColor());
        g.fillRect(
            int(std::lround(cx - shaftW * 0.5f)),
            int(std::lround(cy - shaftH * 0.5f)),
            int(std::lround(shaftW)),
            int(std::lround(shaftH))
        );

        g.setStroke(bodyStroke->getColor());
        g.strokeRect(Rect<int>(
            int(std::lround(cx - shaftW * 0.5f)),
            int(std::lround(cy - shaftH * 0.5f)),
            int(std::lround(shaftW)),
            int(std::lround(shaftH))
        ));
    }
};

REGISTER_ELEMENT_TYPE("throttle", ThrottleElement);