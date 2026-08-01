#pragma once

#include <multigauge/editor/Api.h>
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/screens/Screen.h>

namespace mg {

class EditorScreen : public Screen {
    private:
        editor::EditorId editorId{};
        editor::NodeId faceId = 0;
        gauge::GaugeFace* lastFace = nullptr;

        gauge::GaugeFace* resolveFace() const;
        void ensureFaceInitialized(RuntimeContext& ctx, gauge::GaugeFace* face);

    public:
        explicit EditorScreen(editor::EditorId editorId = editor::EditorId{}, editor::NodeId faceId = 0);

        void setFace(editor::EditorId editorId, editor::NodeId faceId);

        void onShow(RuntimeContext& ctx) override;
        void onHide(RuntimeContext& ctx) override;
        void update(RuntimeContext& ctx, std::chrono::microseconds delta) override;
        void draw(RuntimeContext& ctx, graphics::Graphics& g) override;
};

}
