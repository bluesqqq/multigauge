#include <multigauge/screens/EditorScreen.h>

namespace mg {

EditorScreen::EditorScreen(editor::EditorId editorId, editor::NodeId faceId)
    : editorId(editorId), faceId(faceId) {
}

void EditorScreen::setFace(editor::EditorId editor, editor::NodeId face) {
    editorId = editor;
    faceId = face;
}

gauge::GaugeFace* EditorScreen::resolveFace() const {
    return editor::getFace(editorId, faceId);
}

void EditorScreen::onShow(RuntimeContext& ctx) {
    if (auto* face = resolveFace()) ctx.prepareEditorFace(editorId, *face);
}

void EditorScreen::onHide(RuntimeContext&) {}

void EditorScreen::update(RuntimeContext& ctx, std::chrono::microseconds delta) {
    gauge::GaugeFace* face = resolveFace();
    if (!face) return;

    ctx.prepareEditorFace(editorId, *face);
    face->update(delta);
}

void EditorScreen::draw(RuntimeContext& ctx, graphics::Graphics& g) {
    gauge::GaugeFace* face = resolveFace();
    if (!face) return;

    ctx.prepareEditorFace(editorId, *face);
    face->layout(g);
    face->draw(g);
}

} // namespace mg
