#pragma once

#include <chrono>
#include <vector>

#include <multigauge/graphics/UserPalette.h>
#include <multigauge/value/Value.h>

namespace mg {
class Value;
}

namespace mg::graphics {

/// A frame-stable, index-addressable copy of the registered values.
/// The Value registry is contiguous and static, so a Value* can be converted to
/// an index without an identifier lookup while drawing.
class ValueSnapshot {
public:
    void refresh() noexcept;
    [[nodiscard]] float value(const ::mg::Value* value, float fallback = 0.0F) const noexcept;

private:
    const ::mg::Value* first_ = nullptr;
    std::vector<float> values_;
};

/// Frame-snapshotted color inputs. The user palette is a stable application
/// preference reference and, by contract, is only changed between frames.
class ColorFrame {
public:
    void refresh(std::chrono::microseconds elapsed, const UserPalette& palette) noexcept;

    [[nodiscard]] std::chrono::microseconds elapsed() const noexcept { return elapsed_; }
    [[nodiscard]] const ValueSnapshot& values() const noexcept { return values_; }
    [[nodiscard]] const UserPalette* palette() const noexcept { return palette_; }

private:
    std::chrono::microseconds elapsed_{};
    ValueSnapshot values_;
    const UserPalette* palette_ = nullptr;
};

} // namespace mg::graphics
