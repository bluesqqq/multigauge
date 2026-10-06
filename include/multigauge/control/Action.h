#pragma once

#include <cstddef>

namespace mg::control {

/// @brief Operations that can be assigned to a host-configured control slot.
enum class Action {
    Next,
    Select,
    Previous,
};

/// @brief A persistent assignment from a control slot to an action.
struct Binding {
    std::size_t portIndex = 0;
    Action action = Action::Next;
};

} // namespace mg::control
