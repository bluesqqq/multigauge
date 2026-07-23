#include <multigauge/screens/EditorScreen.h>

namespace mg {

namespace editor {
gauge::GaugeFace* getFace(EditorId editorId, NodeId faceId);
}

EditorScreen::EditorScreen(editor::EditorId editorId, editor::NodeId faceId)
    : editorId(editorId), faceId(faceId) {}

void EditorScreen::setFace(editor::EditorId newEditorId, editor::NodeId newFaceId) {
    editorId = newEditorId;
    faceId = newFaceId;
    lastFace = nullptr;
}

gauge::GaugeFace* EditorScreen::resolveFace() const {
    return editor::getFace(editorId, faceId);
}

void EditorScreen::ensureFaceInitialized(RuntimeContext& ctx, gauge::GaugeFace* face) {
    if (!face || face == lastFace) return;

    face->init(ctx.getAssetManager(), ctx.getGraphicsContext());
    lastFace = face;
}

void EditorScreen::onShow(RuntimeContext& ctx) {
    ensureFaceInitialized(ctx, resolveFace());
}

void EditorScreen::onHide(RuntimeContext& ctx) {
    lastFace = nullptr;
}

void EditorScreen::update(RuntimeContext& ctx, uint64_t deltaUs) {
    gauge::GaugeFace* face = resolveFace();
    if (!face) {
        lastFace = nullptr;
        return;
    }

    ensureFaceInitialized(ctx, face);
    face->update(deltaUs);
}

void EditorScreen::draw(RuntimeContext& ctx, graphics::Graphics& g) {
    gauge::GaugeFace* face = resolveFace();
    if (!face) {
        lastFace = nullptr;
        return;
    }

    ensureFaceInitialized(ctx, face);
    face->layout(g);
    face->draw(g);
}

}
