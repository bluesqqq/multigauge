#include <multigauge/gauge/ticks/TickList.h>

namespace mg::gauge {

float TickList::getLength(uint8_t index) const {
    while (index > 0) {
        if (subs[index - 1].length.has_value()) return subs[index - 1].length.value();
        --index;
    }
    return root.length;
}

float TickList::getThickness(uint8_t index) const {
    while (index > 0) {
        if (subs[index - 1].thickness.has_value()) return subs[index - 1].thickness.value();
        --index;
    }
    return root.thickness;
}

const TickStyle &TickList::getStyle(uint8_t index) const {
    while (index > 0) {
        if (subs[index - 1].style.has_value()) return subs[index - 1].style.value();
        --index;
    }
    return root.style;
}

const PaintTimeline &TickList::getColor(uint8_t index) const {
    while (index > 0) {
        if (subs[index - 1].paint.has_value()) return subs[index - 1].paint.value();
        --index;
    }
    return root.paint;
}

const std::optional<TickValueStyle>& TickList::getValueStyle(uint8_t index) const {
    if (index == 0) return root.valueStyle;

    return subs[index - 1].valueStyle;
}

void TickList::drawCircularTick(Graphics &g, uint8_t index, Point<float> pos, float radius, float position, float angle, float value) const {
    float highlightFactor = getHighlightFactor(value);

    float length    = getLength(index);
    float thickness = getThickness(index);
    const TickStyle& style = getStyle(index);
    const PaintTimeline& color = getColor(index);
    const std::optional<TickValueStyle>& valueStyle = getValueStyle(index);

    if (highlightFactor >= 0) {
        length    *= 1.0f + (lengthFactor * highlightFactor);
        thickness *= 1.0f + (thicknessFactor * highlightFactor);
    }
    
    std::pair<float, float> tickRadii = alignLength(radius, length, align);

    Point<float> unitVector = Point<float>::getPointOnUnitCircle(angle);
    Line<float> line = Line<float>(pos + unitVector * tickRadii.first, pos + unitVector * tickRadii.second);

    // Get the color at the current position  
    const ResolvedPaint temporaryFS = color.sample(value, g.colorFrameToken());

    switch(style) {
        case LINE:
            drawLineTick(g, line, thickness, temporaryFS);
            break;

        case TRIANGLE:
            switch(align) {
                case INNER: // Triangle points outwards
                    drawTriangleTick(g, line, thickness, temporaryFS);
                    break;
                case CENTER: // Triangles points both ways
                    drawTriangleTick(g, line, thickness, temporaryFS);
                    break;

                case OUTER: // Triangle points inwards
                    drawTriangleTick(g, line, thickness, temporaryFS);
                    break;
            }
            break;

        case CIRCLE:
            drawCircleTick(g, line, thickness, temporaryFS);
            break;
    }
}

} // namespace mg::gauge
