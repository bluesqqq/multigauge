#include <multigauge/gauge/ticks/TickList.h>

namespace mg::gauge {
float TickList::getLength(uint8_t index) const {
    while (index > 0) {
        if (subs[index - 1].length) return *subs[index - 1].length;
        --index;
    }
    return root.length;
}

float TickList::getThickness(uint8_t index) const {
    while (index > 0) {
        if (subs[index - 1].thickness) return *subs[index - 1].thickness;
        --index;
    }
    return root.thickness;
}

const TickStyle& TickList::getStyle(uint8_t index) const {
    while (index > 0) {
        if (subs[index - 1].style) return *subs[index - 1].style;
        --index;
    }
    return root.style;
}

const PaintTimeline& TickList::getColor(uint8_t index) const {
    while (index > 0) {
        if (subs[index - 1].paint) return *subs[index - 1].paint;
        --index;
    }
    return root.paint;
}

const std::optional<TickValueStyle>& TickList::getValueStyle(uint8_t index) const {
    return index == 0 ? root.valueStyle : subs[index - 1].valueStyle;
}

void TickList::drawCircularTick(Graphics& g,
                                uint8_t index,
                                Point<float> pos,
                                float radius,
                                float,
                                float angle,
                                float value) const {
    float highlight = getHighlightFactor(value), length = getLength(index),
          thickness = getThickness(index);
    if (highlight >= 0) {
        length *= 1.f + lengthFactor * highlight;
        thickness *= 1.f + thicknessFactor * highlight;
    }
    const auto radii = alignLength(radius, length, align);
    const auto unit = Point<float>::getPointOnUnitCircle(angle);
    const Line<float> line(pos + unit * radii.first, pos + unit * radii.second);
    const auto paint = getColor(index).sample(value, g.colorFrameToken());
    switch (getStyle(index)) {
    case LINE:
        drawLineTick(g, line, thickness, paint);
        break;
    case TRIANGLE:
        drawTriangleTick(g, line, thickness, paint);
        break;
    case CIRCLE:
        drawCircleTick(g, line, thickness, paint);
        break;
    }
}
} // namespace mg::gauge
