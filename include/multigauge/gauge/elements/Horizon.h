#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/StaticColor.h>

class Horizon : public Element {
    private:
        int horizonDensityVertical = 25; // Density variables can be changed to show more/less lines
        int horizonDensityHorizontal = 12;
        int horizonVAngle = 8; // Angle variables have nothing to do with degrees, just a general number to multiply by
        int horizonHAngle = 8;

        float zVelMultiplier = 0.001;
        float xVelMultiplier = 0.003;

        float zPosition = 0;
        float xPosition = 0;

        OwnedColor backgroundColor = nullptr;
        OwnedColor groundColor     = nullptr;
        OwnedColor horizonColor    = nullptr;
        OwnedColor borderColor     = nullptr;

        MG_EDITABLE_BEGIN()
            MG_PROP(backgroundColor)
            MG_PROP(groundColor)
            MG_PROP(horizonColor)
            MG_PROP(borderColor)
        MG_EDITABLE_END()
        
    public:
        using Element::Element;
        
        void draw(Graphics& g) const override {
            const auto b = getBounds().toInt();

            const int left   = b.getLeft();
            const int top    = b.getTop();
            const int right  = b.getRightPixel();
            const int bottom = b.getBottomPixel();

            const int halfY = b.getCenterY();

            auto ground = Rect<int>(b);
            ground.setTop(halfY);
            ground.reduce(1);

            auto background = Rect<int>(b);
            background.setBottom(halfY);
            background.reduce(1);

            if (groundColor) {
                g.setPaint(groundColor.get());
                g.drawRect(ground);
            }

            if (backgroundColor) {
                g.setPaint(backgroundColor.get());
                g.drawRect(background);
            }

            if (horizonColor) {
                g.setPaint(horizonColor.get());

                // Horizon line (middle)
                g.drawLine(left, halfY, right, halfY);

                // Vertical lines
                const float stepX = b.width / float(horizonDensityVertical);
                const float shift = fmodf(xPosition, 1.0f) * stepX;

                for (float i = 0; i < b.width; i += stepX) {
                    float posX = left + i + shift;

                    float localX = (posX - left);
                    float localHalf = b.width / 2.0f;

                    float endXLocal = (localX - localHalf) * horizonVAngle + localHalf;
                    float endX = left + endXLocal;

                    Line<float> raw(posX, (float)halfY, endX, (float)bottom);

                    if (auto clipped = raw.intersection(b.toFloat())) {
                        auto li = clipped->toInt();
                        g.drawLine(li.p1.x, li.p1.y, li.p2.x, li.p2.y);
                    }
                }

                // Horizontal lines
                const float maxValue = powf((float)horizonDensityHorizontal, (float)horizonHAngle);

                for (int i = 0; i < horizonDensityHorizontal; i++) {
                    float pos = (float)i + fmodf(zPosition, 1.0f);

                    float localHalf = b.height / 2.0f;
                    int y = (int)(halfY + powf(pos, (float)horizonHAngle) * (localHalf / maxValue));

                    g.drawLine(left, y, right, y);
                }
            }

            if (borderColor) {
                g.setPaint(nullptr, borderColor.get(), 1.0f);
                g.drawRect(b);
            }
        }

        void update(int deltaTime) override {
            zPosition += 0; //lateralAcceleration.getValueRaw() * zVelMultiplier; //zValue.getValue(DEFAULT) * zVelMultiplier;
            xPosition -= 0.01f; //speed.getValueRaw() * xVelMultiplier; //xValue.getValue(DEFAULT) * xVelMultiplier;
        }
};