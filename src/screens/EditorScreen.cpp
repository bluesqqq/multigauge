#include <multigauge/screens/EditorScreen.h>

#include "../editor/EditorPreview.h"

#include <multigauge/editor/Editor.h>
#include <multigauge/runtime/RuntimeContext.h>

namespace mg {

EditorScreen::EditorScreen(editor::EditorId editorId, editor::NodeId faceId)
    : editorId(editorId), faceId(faceId), preview(std::make_unique<editor::EditorPreview>()) {
}

EditorScreen::~EditorScreen() = default;

void EditorScreen::setFace(editor::EditorId editor, editor::NodeId face) {
    editorId = editor;
    faceId = face;
}

gauge::GaugeFace* EditorScreen::resolveFace() const {
    return editor::getFace(editorId, faceId);
}

void EditorScreen::onShow(RuntimeContext& ctx) {
    if (auto* face = resolveFace()) preview->prepare(ctx, editorId, faceId, *face);
}

void EditorScreen::onHide(RuntimeContext&) {}

void EditorScreen::update(RuntimeContext& ctx, std::chrono::microseconds delta) {
    gauge::GaugeFace* face = resolveFace();
    if (!face) return;

    preview->prepare(ctx, editorId, faceId, *face);
    face->update(delta);
}

void EditorScreen::draw(RuntimeContext& ctx, graphics::Graphics& g) {
    gauge::GaugeFace* face = resolveFace();
    if (!face) return;

    preview->prepare(ctx, editorId, faceId, *face);
    face->layout(g);
    face->draw(g);
}

} // namespace mg
