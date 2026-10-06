#pragma once

#include <string>
#include <string_view>

#include <multigauge/Config.h>
#include <multigauge/control/Action.h>
#include <multigauge/control/Result.h>
#include <multigauge/context/Manager.h>
#if MG_BUILD_EDITOR
#include <multigauge/editor/Types.h>
#endif

namespace mg {

namespace package { class Manager; }
#if MG_BUILD_EDITOR
namespace editor { class Manager; }
#endif

namespace navigation {

/// @brief Selects and transitions screens within registered contexts.
class Manager {
public:
    Manager(
        context::Manager& contexts,
        package::Manager& packages
#if MG_BUILD_EDITOR
        , editor::Manager& editors
#endif
    );

    /// @brief Shows a gauge screen created from a face JSON document.
    [[nodiscard]] bool showGauge(ContextId id, const std::string& json);

    /// @brief Shows an installed package face as a gauge screen.
    [[nodiscard]] bool showGauge(
        ContextId id,
        std::string_view packageId,
        std::string_view faceId
    );

#if MG_BUILD_EDITOR
    /// @brief Shows an editor screen for an editor face.
    [[nodiscard]] bool showEditor(ContextId id, editor::EditorId editorId, editor::NodeId faceId);
#endif

    /// @brief Offers a bound control action to the active screen and applies its result.
    [[nodiscard]] control::Result handleControl(
        ContextId id,
        const control::Action& action
    );

private:
    [[nodiscard]] bool cycleGaugeFace(ContextId id, int offset);

    context::Manager& contexts_;
    package::Manager& packages_;
#if MG_BUILD_EDITOR
    editor::Manager& editors_;
#endif
};

} // namespace navigation

} // namespace mg
