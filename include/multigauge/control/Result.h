#pragma once

namespace mg::control {

/// @brief The outcome requested by the active screen for a control action.
struct Result {
    enum class Type {
        Ignored,
        Handled,
        NextFace,
        PreviousFace,
        OpenMenu,
    };

    Type type = Type::Ignored;

    [[nodiscard]] static Result ignored() { return {}; }
    [[nodiscard]] static Result handled() { return {Type::Handled}; }
    [[nodiscard]] static Result nextFace() { return {Type::NextFace}; }
    [[nodiscard]] static Result previousFace() { return {Type::PreviousFace}; }
    [[nodiscard]] static Result openMenu() { return {Type::OpenMenu}; }
};

} // namespace mg::control
