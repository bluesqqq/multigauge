#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <multigauge/editor/Types.h>

namespace mg {

// Forward declarations
class RuntimeContext;
namespace gauge { class GaugeFace; }

namespace editor {

/// Synchronizes editor-owned assets and resources for one rendered preview.
class EditorPreview {
public:
    void prepare(RuntimeContext& context, EditorId editorId, NodeId faceId, ::mg::gauge::GaugeFace& face);

private:
    static constexpr std::size_t invalidRevision = static_cast<std::size_t>(-1);

    EditorId activeEditor{};
    NodeId activeFace = 0;
    std::size_t appliedAssetRevision = invalidRevision;
    std::size_t appliedRevision = invalidRevision;
    std::vector<std::string> stagedAssetNames;
};

} // namespace editor

} // namespace mg
