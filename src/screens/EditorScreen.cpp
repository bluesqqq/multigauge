#include <multigauge/screens/EditorScreen.h>

#include "../editor/Preview.h"

#include <multigauge/editor/Editor.h>
#include <multigauge/editor/Manager.h>
#include <multigauge/context/Context.h>

namespace mg {

EditorScreen::EditorScreen(
    editor::Manager& editors,
    editor::EditorId editorId,
    editor::NodeId faceId
) : editors(editors),
    editorId(editorId),
    faceId(faceId),
    preview(std::make_unique<editor::Preview>()) {}

EditorScreen::~EditorScreen() = default;

void EditorScreen::setFace(editor::EditorId editor, editor::NodeId face) {
    editorId = editor;
    faceId = face;
}

gauge::GaugeFace* EditorScreen::resolveFace() const {
    return editor::getFace(editors, editorId, faceId);
}

void EditorScreen::onShow(context::Context& ctx) {
    if (auto* face = resolveFace()) preview->prepare(editors, ctx, editorId, faceId, *face);
}

void EditorScreen::onHide(context::Context&) {}

void EditorScreen::update(context::Context& ctx, std::chrono::microseconds delta) {
    gauge::GaugeFace* face = resolveFace();
    if (!face) return;

    preview->prepare(editors, ctx, editorId, faceId, *face);
    face->update(delta);
}

void EditorScreen::draw(context::Context& ctx, graphics::Graphics& g) {
    gauge::GaugeFace* face = resolveFace();
    if (!face) return;

    preview->prepare(editors, ctx, editorId, faceId, *face);
    face->layout(g);
    face->draw(g);
}

} // namespace mg
