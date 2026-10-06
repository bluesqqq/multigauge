#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <vector>

#include <multigauge/control/Action.h>
#include <multigauge/control/Result.h>
#include <multigauge/json/Json.h>
#include <multigauge/navigation/Manager.h>

#ifndef MG_CONTROL_MAX_PORTS
#error "MG_CONTROL_MAX_PORTS must be defined by the target build."
#endif

namespace mg {
namespace io { class FileSystem; }
}

namespace mg::control {

/// @brief Owns fixed control slots and their persisted action bindings.
/// @details Hosts configure a slot count, bind actions, save them, and call
/// trigger when the matching physical input activates.
class Manager {
public:
    static constexpr std::size_t MaxPorts = MG_CONTROL_MAX_PORTS;
    static_assert(MaxPorts > 0, "MG_CONTROL_MAX_PORTS must be positive.");

    Manager(
        io::FileSystem& fs,
        std::string dataRoot,
        navigation::Manager& navigation
    );

    /// @brief Loads control bindings from device-local storage.
    [[nodiscard]] bool load();

    /// @brief Writes changed control bindings to device-local storage.
    [[nodiscard]] bool save();

    /// @brief The number of fixed control slots selected by the target build.
    [[nodiscard]] std::size_t portCount() const noexcept;

    /// @brief Creates or replaces an action binding for a control slot.
    [[nodiscard]] bool bind(std::size_t portIndex, Action action);

    /// @brief Removes the binding for a control slot.
    [[nodiscard]] bool unbind(std::size_t portIndex);

    /// @brief Lists every configured port-to-action binding.
    void listBindings(std::vector<Binding>& out) const;

    /// @brief Invokes a slot's action for a host-selected local gauge context.
    /// @return The active screen's result, or `Ignored` if the slot cannot trigger.
    [[nodiscard]] Result trigger(ContextId contextId, std::size_t portIndex);

private:
    struct Slot {
        Action action = Action::Next;
        bool bound = false;
    };

    [[nodiscard]] Slot* slotAt(std::size_t portIndex) noexcept;
    [[nodiscard]] const Slot* slotAt(std::size_t portIndex) const noexcept;
    [[nodiscard]] bool loadDocument(json::Reader root);
    [[nodiscard]] bool writeDocument(json::Writer& writer) const;

    io::FileSystem& fs_;
    std::string dataRoot_;
    navigation::Manager& navigation_;
    std::array<Slot, MaxPorts> slots_{};
    bool dirty_ = false;
};

} // namespace mg::control
