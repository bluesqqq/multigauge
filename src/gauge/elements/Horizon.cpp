#include <cmath>
#include <multigauge/gauge/elements/Horizon.h>
#include <multigauge/graphics/Graphics.h>

namespace mg::gauge {
void Horizon::draw(::mg::graphics::Graphics& g, const ::mg::Rect<float>& bounds) const {
    const auto b = bounds.toInt();
    const int left = b.getLeft();
    const int right = b.getRightPixel();
    const int bottom = b.getBottomPixel();
    const int middle = b.getCenterY();
    auto ground = b;
    ground.setTop(middle);
    ground.reduce(1);
    auto background = b;
    background.setBottom(middle);
    background.reduce(1);

    if (groundColor_) {
        g.setPaint(groundColor_.get());
        g.drawRect(ground);
    }
    if (backgroundColor_) {
        g.setPaint(backgroundColor_.get());
        g.drawRect(background);
    }
    if (horizonColor_) {
        g.setPaint(horizonColor_.get());
        g.drawLine(left, middle, right, middle);

        const float verticalStep = b.width / static_cast<float>(horizonDensityVertical_);
        const float shift = std::fmod(xPosition_, 1.0f) * verticalStep;
        for (float offset = 0; offset < b.width; offset += verticalStep) {
            const float x = left + offset + shift;
            const float localX = x - left;
            const float localMiddle = b.width / 2.0f;
            const float endX = left + (localX - localMiddle) * horizonVAngle_ + localMiddle;
            const ::mg::Line<float> raw(x, static_cast<float>(middle), endX, static_cast<float>(bottom));
            if (const auto clipped = raw.intersection(b.toFloat())) {
                const auto line = clipped->toInt();
                g.drawLine(line.p1.x, line.p1.y, line.p2.x, line.p2.y);
            }
        }

        const float maximum = std::pow(
            static_cast<float>(horizonDensityHorizontal_),
            static_cast<float>(horizonHAngle_)
        );
        for (int index = 0; index < horizonDensityHorizontal_; ++index) {
            const float position = index + std::fmod(zPosition_, 1.0f);
            const float localMiddle = b.height / 2.0f;
            const int y = static_cast<int>(middle + std::pow(
                position, static_cast<float>(horizonHAngle_)
            ) * (localMiddle / maximum));
            g.drawLine(left, y, right, y);
        }
    }
    if (borderColor_) {
        g.setPaint(nullptr, borderColor_.get());
        g.drawRect(b);
    }
}

void Horizon::update(std::chrono::microseconds) {
    xPosition_ -= 0.01f;
}
} // namespace mg::gauge
