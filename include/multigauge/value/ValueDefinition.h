#pragma once

#include <multigauge/value/UnitType.h>

#include <string_view>

namespace mg {

/// Immutable metadata for a registered value.
struct ValueDefinition {
    std::string_view id;
    std::string_view name;
    const UnitType* unit = nullptr;
    Measurement minimum = 0.0F;
    Measurement maximum = 0.0F;
};

} // namespace mg