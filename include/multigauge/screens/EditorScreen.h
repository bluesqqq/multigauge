#pragma once

#include <multigauge/editor/Api.h>
#include <multigauge/screens/Screen.h>

namespace mg {

class EditorScreen : public Screen {
    private:
        editor::EditorId editorId = 0;
        editor::Editor::Id faceId = 0;
        gauge::GaugeFace* lastFace = nullptr;

        gauge::GaugeFace* resolveFace() const;
        void ensureFaceInitialized(RuntimeContext& ctx, gauge::GaugeFace* face);

    public:
        explicit EditorScreen(editor::EditorId editorId = 0, editor::Editor::Id faceId = 0);

        void setFace(editor::EditorId editorId, editor::Editor::Id faceId);

        void onShow(RuntimeContext& ctx) override;
        void onHide(RuntimeContext& ctx) override;
        void update(RuntimeContext& ctx, uint64_t deltaUs) override;
        void draw(RuntimeContext& ctx, graphics::Graphics& g) override;
};

}
