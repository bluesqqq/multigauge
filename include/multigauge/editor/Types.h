#pragma once

#include <cstddef>
#include <cstdint>

#include <multigauge/container/GenerationalHandle.h>
#include <multigauge/gauge/Element.h>

namespace mg::editor {

using EditorId = GenerationalHandle<struct EditorTag>;
using NodeId = uint32_t;

/// @brief Specifies the destination index for a face operation.
struct FacePlacement {
    std::size_t index;

    /// @brief Creates a placement at an index, appending by default.
    explicit FacePlacement(std::size_t i = static_cast<std::size_t>(-1)) : index(i) {}
};

/// @brief Stable reference to an element within one editor face.
struct ElementRef {
    NodeId faceId = 0;
    gauge::NodeHandle handle;
};

/// @brief Specifies an element's face, parent, and sibling index.
struct ElementPlacement {
    NodeId faceId = 0;
    gauge::NodeHandle parent;
    std::size_t index = static_cast<std::size_t>(-1);

    /// @brief Creates an element placement, appending by default.
    ElementPlacement(
        NodeId face = 0,
        gauge::NodeHandle parentHandle = {},
        std::size_t i = static_cast<std::size_t>(-1)
    )
        : faceId(face), parent(parentHandle), index(i) {}
};

} // namespace mg::editor
