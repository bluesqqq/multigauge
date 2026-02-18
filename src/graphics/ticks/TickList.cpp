#include "TickList.h"

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

const FillStrokeTimeline &TickList::getColor(uint8_t index) const {
    while (index > 0) {
        if (subs[index - 1].color.has_value()) return subs[index - 1].color.value();
        --index;
    }
    return root.color;
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
    const FillStrokeTimeline& color = getColor(index);
    const std::optional<TickValueStyle>& valueStyle = getValueStyle(index);

    if (highlightFactor >= 0) {
        length    *= 1.0f + (lengthFactor * highlightFactor);
        thickness *= 1.0f + (thicknessFactor * highlightFactor);
    }
    
    std::pair<float, float> tickRadii = alignLength(radius, length, align);

    Point<float> unitVector = Point<float>::getPointOnUnitCircle(angle);
    Line<float> line = Line<float>(pos + unitVector * tickRadii.first, pos + unitVector * tickRadii.second);

    // Get the color at the current position  
    FillStroke temporaryFS = color.getFillStrokeAtPosition(value);

    //if (fillStrokeModifier.fill != nullptr) temporaryFS.blendFill(fillStrokeModifier.fill->getColor(), highlightFactor);
    //if (fillStrokeModifier.stroke != nullptr) temporaryFS.blendStroke(fillStrokeModifier.stroke->getColor(), fillStrokeModifier.stroke->thickness, highlightFactor);

    /*
    if (fade != nullptr) { // Apply fade
        uint8_t alpha = fade->getAlpha(position);
        temporaryFS = temporaryFS.getBlendFillStroke(fade->color->getColor(), alpha);
    }
    */

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
   
    /*
    // Draw the value if needed
    if (valueStyle.has_value()) {
        double cosAngle, sinAngle; sincos(angle, &sinAngle, &cosAngle);

        TickValueStyle style = valueStyle.value();

        String displayValue = String((int)round(value));

        // Set text parameters
        canvas.setTextDatum(middle_center);
        canvas.setTextFont(style.font);
        canvas.setTextSize(1.0f * (1.0f + (highlightFactor * textSizeFactor)));
        
        uint16_t valueColor = style.color.getColor(value);

        valueColor = blendColors(valueColor, valueColorModifier->getColor(), highlightFactor);

        if (fade != nullptr) { // Apply fade
            uint8_t alpha = fade->getAlpha(position);
            canvas.setTextColor(alphaBlend(alpha, valueColor, fade->color->getColor()));
        }

        // Calculate the drawn values width and height
        int stringWidth = canvas.textWidth(displayValue);
        int stringHeight = canvas.fontHeight();

        // Check if the alignment is RADIUS_INNER or if flip is enabled, but not both
        // Also an excuse to use the cool '^' XOR operator
        bool isInnerFlipped = (alignment == LENGTH_INNER) ^ style.flipValuesPosition;

        int modifierSign = isInnerFlipped ? 1 : -1;

        // Some simple math to determine a height/width modifier to draw the values at
        int widthModifier  = cosAngle * ((stringWidth / 2)  + style.spacing) * modifierSign;
        int heightModifier = sinAngle * ((stringHeight / 2) + style.spacing) * modifierSign;

        float selectedRadii = isInnerFlipped ? tickRadii.second : tickRadii.first;

        canvas.drawString(displayValue, pos.x + selectedRadii * cosAngle + widthModifier, pos.y + selectedRadii * sinAngle + heightModifier);
    }
    */
}