#include "EditorPreview.h"
#include <multigauge/editor/EditorRegistry.h>

#include <multigauge/editor/Editor.h>
#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/runtime/RuntimeContext.h>

#include <algorithm>

namespace mg::editor {

void EditorPreview::prepare(RuntimeContext& context, EditorId editorId, NodeId faceId, ::mg::gauge::GaugeFace& face) {
    Editor* value = find(editorId);
    if (!value) return;

    const auto& editorAssets = value->assets();
    const std::size_t assetRevision = value->assetRevision();
    const std::size_t revision = value->revision();
    auto& assets = context.getAssetManager();

    if (editorId != activeEditor || assetRevision != appliedAssetRevision) {
        std::vector<Editor::AssetChange> changes;
        bool fullSync = activeEditor != editorId || appliedAssetRevision == invalidRevision ||
                        !value->assetChangesSince(appliedAssetRevision, changes);
        if (!fullSync) {
            fullSync = std::any_of(changes.begin(), changes.end(), [](const auto& change) {
                return change.kind == Editor::AssetChange::Kind::Reset;
            });
        }

        if (fullSync) {
            for (const std::string& name : stagedAssetNames) {
                const auto current = std::find_if(editorAssets.begin(), editorAssets.end(), [&](const auto& asset) {
                    return asset.name == name;
                });
                if (current == editorAssets.end()) (void)assets.removeAsset({}, name);
            }
            stagedAssetNames.clear();
            for (const auto& asset : editorAssets) {
                if (assets.writeAsset({}, asset.name, asset.data)) stagedAssetNames.push_back(asset.name);
            }
        } else {
            for (const auto& change : changes) {
                if (change.kind == Editor::AssetChange::Kind::Remove) {
                    (void)assets.removeAsset({}, change.asset.name);
                    stagedAssetNames.erase(std::remove(stagedAssetNames.begin(), stagedAssetNames.end(), change.asset.name),
                                           stagedAssetNames.end());
                } else if (change.kind == Editor::AssetChange::Kind::Upsert &&
                           assets.writeAsset({}, change.asset.name, change.asset.data) &&
                           std::find(stagedAssetNames.begin(), stagedAssetNames.end(), change.asset.name) == stagedAssetNames.end()) {
                    stagedAssetNames.push_back(change.asset.name);
                }
            }
        }
    }

    if (activeEditor != editorId || activeFace != faceId || appliedRevision != revision) {
        face.init({}, assets, context.getGraphicsContext());
        activeEditor = editorId;
        activeFace = faceId;
        appliedRevision = revision;
    }
    appliedAssetRevision = assetRevision;
}

} // namespace mg::editor
