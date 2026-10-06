#pragma once

#include <string>

namespace mg::control {

/// @brief Operations that can be assigned to an exposed control port.
enum class Action {
    Next,
    Select,
    Previous,
};

/// @brief A persistent assignment from a host-defined control port to an action.
struct Binding {
    std::string portId;
    Action action = Action::Next;
};

} // namespace mg::control
