#pragma once

#include <string>

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

struct ClipboardSummary {
    ClipboardState::Kind kind = ClipboardState::Kind::Empty;

    bool hasValue() const {
        return kind != ClipboardState::Kind::Empty;
    }
};
