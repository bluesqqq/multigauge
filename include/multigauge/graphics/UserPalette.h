#pragma once

#include <array>
#include <cstddef>

#include <multigauge/graphics/colors/rgba.h>

namespace mg::graphics {

/// Persistent user preference colors shared by every runtime context.
/// Palette updates must occur outside drawing; UserColor definitions only store
/// an index into this palette.
class UserPalette {
public:
    static constexpr std::size_t Size = 3;

    [[nodiscard]] rgba color(std::size_t index, rgba fallback = rgba{0, 0, 0, 0}) const noexcept {
        return index < colors_.size() ? colors_[index] : fallback;
    }

    bool setColor(std::size_t index, rgba color) noexcept {
        if (index >= colors_.size()) return false;
        colors_[index] = color;
        return true;
    }

private:
    std::array<rgba, Size> colors_{
        rgba{255, 255, 255, 255},
        rgba{255, 0, 0, 255},
        rgba{0, 0, 0, 255},
    };
};

} // namespace mg::graphics
