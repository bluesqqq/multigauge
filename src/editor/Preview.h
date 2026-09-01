#pragma once

#include <cstddef>
#include <string>
#include <vector>

#include <multigauge/editor/Types.h>

namespace mg {
namespace context { class Context; }
namespace gauge { class GaugeFace; }
namespace editor {

class Manager;

/// Synchronizes editor-owned assets and resources for one rendered preview.
class Preview {
public:
    void prepare(Manager& editors, context::Context& context, EditorId editorId, NodeId faceId,
                 ::mg::gauge::GaugeFace& face);

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
