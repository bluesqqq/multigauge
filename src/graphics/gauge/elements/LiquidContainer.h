#include "graphics/gauge/Element.h"
#include "graphics/colors/StaticColor.h"
#include "graphics/DisplayValue.h"

class LiquidPoint {
    public:
        float y = 0;
        float velocity = 0;

        void targetNeighbor(LiquidPoint* neighbor, float springConstant, float damping) {
            // Spring force calculation towards target point and neighboring points
            float forceNeighbor = springConstant * (neighbor->y - y);

            // Update velocities
            velocity += forceNeighbor- (velocity * damping);
        }

        void target(float target, float springConstant, float damping) {
            // Spring force calculation towards target point and neighboring points
            float forceTarget = springConstant * (target - y);

            // Update velocities
            velocity += forceTarget - (velocity * damping);
        }

        void update() {
            y += velocity;
        }

        LiquidPoint()
        {

        }
};

class LiquidContainer : public Element {
    private:
        DisplayValue value;
        Value& rollValue;
        Value& pitchValue;

        std::vector<LiquidPoint> points;
        
        Color* liquidColor;
        Color* backgroundColor;
        Color* outlineColor;

        /// @brief Time variable to be used for the liquid simulation.
        float t = 0;

        float lastAngle = 0;
        float lastAngleVelocity = 0;
        float lastAngleAcceleration = 0;

        float percentCached = 0.0f;
        float spacingCached = 0.0f;
        float centerYCached = 0.0f;
        float forwardAmountCached = 0.0f;
        float maxYCached = 0.0f;
        rgba shadedColorCached = {0, 0, 0, 255};
        int leftCached = 0;
        int topCached = 0;
        int widthCached = 0;
        int heightCached = 0;

    public:
        LiquidContainer(Element* parent, const rapidjson::Value::ConstObject json) : Element(parent, json), rollValue(dummy), pitchValue(dummy) { }

        void draw(Graphics& g) const override {
            const auto b = getBounds().toInt();

            const float spacing       = spacingCached;
            const float forwardAmount = forwardAmountCached;
            const float maxY          = maxYCached;
            const rgba shadedColor = shadedColorCached;

            const int left   = b.getLeft();
            const int top    = b.getTop();
            const int width  = b.width;
            const int height = b.height;

            // Background (your old code was drawRect; if you want filled background, use fillRect)
            g.setStroke(backgroundColor->getColor());
            g.strokeRect(b);

            // ----- TOP OF LIQUID (shaded) -----
            for (size_t i = 0; i < points.size() - 1; ++i) {
                const float x1 = left + i * spacing;
                const float x2 = left + (i + 1) * spacing;

                const float y1 = constrain(top + points[i].y     - forwardAmount, (float)top, maxY);
                const float y2 = constrain(top + points[i + 1].y - forwardAmount, (float)top, maxY);

                // Fill the "top face" wedge
                g.setFill(shadedColor);

                if (points[i].y > points[i + 1].y) {
                    // sloping upwards left
                    g.fillTri(x1, y1, x2, y2, x2, y1);

                    const float h = std::min(forwardAmount, float((top + height) - y1 - 1));
                    g.fillRect(x1, y1, spacing + 1, h);
                } else {
                    g.fillTri(x1, y1, x2, y2, x1, y2);

                    const float h = std::min(forwardAmount, float((top + height) - y2 - 1));
                    g.fillRect(x1, y2, spacing + 1, h);
                }

                // Outline of top surface
                g.setStroke(liquidColor->getColor());
                g.strokeLine(x1, y1, x2, y2);
            }

            // ----- FRONT OF LIQUID (main body) -----
            for (size_t i = 0; i < points.size() - 1; ++i) {
                const float x1 = left + i * spacing;
                const float x2 = left + (i + 1) * spacing;

                const float y1w = constrain(top + points[i].y,     (float)top, maxY);
                const float y2w = constrain(top + points[i + 1].y, (float)top, maxY);

                g.setFill(liquidColor->getColor());

                if (points[i].y > points[i + 1].y) {
                    g.fillTri(x1, y1w, x2, y2w, x2, y1w);

                    const float h = std::min(height - points[i].y, float((top + height) - y1w - 1));
                    g.fillRect(x1, y1w, spacing + 1, h);
                } else {
                    g.fillTri(x1, y1w, x2, y2w, x1, y2w);

                    const float h = std::min(height - points[i].y, float((top + height) - y2w - 1));
                    g.fillRect(x1, y2w, spacing + 1, h);
                }

                // Front outline
                g.setStroke(outlineColor->getColor());
                g.strokeLine(x1, y1w, x2, y2w);
            }

            // Container outline
            g.setStroke(outlineColor->getColor());
            g.strokeRect(b);

            g.setFill({0,0,0,0});
            g.drawText(value.getValueString(DEFAULT_UNIT), b.getCenter().translated(2,2), Anchor::Center);

            g.setFill({255,255,255,255});
            g.drawText(value.getValueString(DEFAULT_UNIT), b.getCenter(), Anchor::Center);
        }


        void update(int deltaTime) override {
            (void)deltaTime;

            const auto b = getBounds().toInt();
            leftCached = b.getLeft();
            topCached = b.getTop();
            widthCached = b.width;
            heightCached = b.height;

            percentCached = value.getInterpolationValue();
            spacingCached = widthCached / float(points.size() - 1);

            const float tiltAngle = constrain(rollValue.getValue(DEFAULT), -60.0f, 60.0f);

            const float angleVelocity = tiltAngle - lastAngle;
            const float angleAcceleration = angleVelocity - lastAngleAcceleration;

            centerYCached = heightCached * (1.0f - percentCached);

            const float springConstant = 0.3f;
            const float damping = 0.1f;

            // Target level (points[i].y is LOCAL within the container, 0..height)
            for (size_t i = 0; i < points.size(); ++i) {
                float target = centerYCached
                    + spacingCached * (i - (points.size() / 2.0f))
                    * tan(tiltAngle * DEG_TO_RAD);

                if (points[i].y > target) points[i].target(target, springConstant, damping);
                else                      points[i].target(target, springConstant * 2.0f, damping);
            }

            // Target neighboring points
            for (size_t i = 0; i < points.size(); ++i) {
                if (i != 0) points[i].targetNeighbor(&points[i - 1], 0.8f, damping);
                if (i != points.size() - 1) points[i].targetNeighbor(&points[i + 1], 0.8f, damping);
            }

            // Integrate
            for (auto& p : points) p.update();

            forwardAmountCached = 20.0f + (pitchValue.getValue(DEFAULT) / 90.0f) * 40.0f;
            shadedColorCached = liquidColor->getColor().blend(rgb(0, 0, 0), 0.5f);

            // This is WORLD max Y for drawing, with your old "-2" quirk.
            maxYCached = float(topCached + heightCached - 2);

            lastAngle = tiltAngle;
            lastAngleVelocity = angleVelocity;
            lastAngleAcceleration = angleAcceleration;

            t += 1.0f;
        }

};

REGISTER_ELEMENT_TYPE("liquid-container", LiquidContainer);