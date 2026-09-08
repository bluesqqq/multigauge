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
        MG_PROP(left, "left")
        MG_PROP(right, "right")
        MG_PROP(top, "top")
        MG_PROP(bottom, "bottom")
    MG_PROPS_END()
};

} // namespace mg::gauge::layout
