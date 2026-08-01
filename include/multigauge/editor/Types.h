#pragma once

#include <cstddef>
#include <cstdint>

#include <multigauge/container/GenerationalHandle.h>

namespace mg::editor {

using EditorId = GenerationalHandle<struct EditorTag>;
using NodeId = uint32_t;

struct FacePlacement {
    std::size_t index;

    explicit FacePlacement(std::size_t i = static_cast<std::size_t>(-1)) : index(i) {}
};

struct ElementPlacement {
    NodeId parentId;
    std::size_t index;

    ElementPlacement(NodeId p = 0, std::size_t i = static_cast<std::size_t>(-1)) : parentId(p), index(i) {}
};

}
