#include "alignment.h"

std::pair<float, float> alignLength(float start, float length, LengthAlignment alignment) {
    switch (alignment) {
        case OUTER:
            return std::pair<float, float>(start, start - length);

        case CENTER: {
            const float halfLength = length / 2.0;
            return std::pair<float, float>(start - halfLength, start + halfLength);
        }

        case INNER:
            return std::pair<float, float>(start, start + length);
        
        default:
            return std::pair<float, float>(start, start);
    }
}