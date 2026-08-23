#pragma once

#include <multigauge/properties/PropertyObject.h>

namespace mg::gauge::layout {

/// @brief Stores a two-dimensional pixel offset.
struct Offset : ::mg::PropertyObject {
    float x = 0.0F;
    float y = 0.0F;

    MG_PROPS_BEGIN()
        MG_PROP(x, "x", "X", "Horizontal pixel offset.")
        MG_PROP(y, "y", "Y", "Vertical pixel offset.")
    MG_PROPS_END()
};

} // namespace mg::gauge::layout
