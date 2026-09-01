#pragma once

#include <memory>

#include <multigauge/editor/Api.h>
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/screens/Screen.h>

namespace mg {
namespace editor { class Manager; class Preview; }

class EditorScreen : public Screen {
public:
    //----------[ CTOR + DTOR ]----------//

    EditorScreen(
        editor::Manager& editors,
        editor::EditorId editorId = editor::EditorId{},
        editor::NodeId faceId = 0
    );

    ~EditorScreen() override;

    //----------[ FACE ]----------//

    void setFace(editor::EditorId editorId, editor::NodeId faceId);

    //----------[ LIFECYCLE ]----------//

    void onShow(context::Context& ctx) override;
    void onHide(context::Context& ctx) override;

    void update(context::Context& ctx, std::chrono::microseconds delta) override;
    void draw(context::Context& ctx, graphics::Graphics& g) override;

private:
    gauge::GaugeFace* resolveFace() const;

private:
    editor::Manager& editors;
    editor::EditorId editorId{};
    editor::NodeId faceId = 0;
    std::unique_ptr<editor::Preview> preview;

};

}
