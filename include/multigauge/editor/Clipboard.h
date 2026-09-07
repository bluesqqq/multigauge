#pragma once

#include <string>

namespace mg::editor {

struct ClipboardState {
    enum class Kind {
        Empty,
        Face,
        Element
    };

    Kind kind = Kind::Empty;
    std::string json;

    void clear() {
        kind = Kind::Empty;
        json.clear();
    }
};

}
