#pragma once

#include <multigauge/properties/PropertyObject.h>

namespace mg::gauge::layout {

/// @brief Stores pixel padding around a layout container's children.
struct Padding : ::mg::PropertyObject {
    int left = 0;
    int right = 0;
    int top = 0;
    int bottom = 0;

    MG_PROPS_BEGIN()
        MG_PROP(left, "left", "Left", "Left child padding in pixels.")
        MG_PROP(right, "right", "Right", "Right child padding in pixels.")
        MG_PROP(top, "top", "Top", "Top child padding in pixels.")
        MG_PROP(bottom, "bottom", "Bottom", "Bottom child padding in pixels.")
    MG_PROPS_END()
};

} // namespace mg::gauge::layout
