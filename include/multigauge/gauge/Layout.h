#pragma once

#include <multigauge/gauge/layout/ChildAlignment.h>
#include <multigauge/gauge/layout/Direction.h>
#include <multigauge/gauge/layout/Floating.h>
#include <multigauge/gauge/layout/Padding.h>
#include <multigauge/gauge/layout/Size.h>
#include <multigauge/properties/PropertyObject.h>

namespace mg::gauge::layout {

/// @brief Stores Clay-native layout state for an element or face.
struct Layout : ::mg::PropertyObject {
    Size width;
    Size height;
    Direction direction = Direction::TopToBottom;
    Padding padding;
    int childGap = 0;
    ChildAlignment childAlignment;
    Floating floating;
    float aspectRatio = 0.0F;

    MG_PROPS_BEGIN()
    MG_PROP(width, "width", "Width", "Clay width sizing.")
    MG_PROP(height, "height", "Height", "Clay height sizing.")
    MG_PROP(direction, "direction", "Direction", "Child layout direction.")
    MG_PROP(padding, "padding", "Padding", "Padding around children.")
    MG_PROP(childGap, "childGap", "Child Gap", "Space between children.")
    MG_PROP(childAlignment, "childAlignment", "Child Alignment", "Cross-axis alignment for children.")
    MG_PROP(floating, "floating", "Floating", "Floating-layer placement.")
    MG_PROP(aspectRatio, "aspectRatio", "Aspect Ratio", "Target width divided by height; zero disables it.")
    MG_PROPS_END()
};

} // namespace mg::gauge::layout
