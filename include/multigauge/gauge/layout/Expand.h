#pragma once

#include <multigauge/properties/PropertyObject.h>

namespace mg::gauge::layout {

/// @brief Stores the expansion of a floating element's outer bounds in pixels.
struct Expand : ::mg::PropertyObject {
    float width = 0.0F;
    float height = 0.0F;

    MG_PROPS_BEGIN()
        MG_PROP(width, "width")
        MG_PROP(height, "height")
    MG_PROPS_END()
};

} // namespace mg::gauge::layout
