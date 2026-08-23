#pragma once

#include <memory>

#include <multigauge/editor/Api.h>
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/screens/Screen.h>

namespace mg {
namespace editor { class EditorPreview; }

class EditorScreen : public Screen {
    private:
        editor::EditorId editorId{};
        editor::NodeId faceId = 0;
        std::unique_ptr<editor::EditorPreview> preview;

        gauge::GaugeFace* resolveFace() const;

    public:
        explicit EditorScreen(editor::EditorId editorId = editor::EditorId{}, editor::NodeId faceId = 0);
        ~EditorScreen() override;

        void setFace(editor::EditorId editorId, editor::NodeId faceId);

        void onShow(RuntimeContext& ctx) override;
        void onHide(RuntimeContext& ctx) override;
        void update(RuntimeContext& ctx, std::chrono::microseconds delta) override;
        void draw(RuntimeContext& ctx, graphics::Graphics& g) override;
};

}
