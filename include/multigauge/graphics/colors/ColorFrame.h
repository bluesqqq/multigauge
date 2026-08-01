#pragma once

#include <chrono>
#include <array>

#include <multigauge/graphics/UserPalette.h>
#include <multigauge/value/ValueRegistry.h>

namespace mg::graphics {

/// Frame-stable measurements keyed by compact value handles. It has fixed
/// capacity and performs no allocation while rendering.
class ValueSnapshot {
public:
    void refresh() noexcept;
    [[nodiscard]] Measurement value(ValueHandle handle, Measurement fallback = 0.0F) const noexcept;
private:
    std::array<Measurement, ValueRegistry::BuiltInCount> builtIns_{};
    std::array<Measurement, ValueRegistry::MaxUserValues> users_{};
    std::array<std::uint16_t, ValueRegistry::MaxUserValues> generations_{};
};

class ColorFrame {
public:
    void refresh(std::chrono::microseconds elapsed, const UserPalette& palette) noexcept;
    [[nodiscard]] std::chrono::microseconds elapsed() const noexcept;
    [[nodiscard]] const ValueSnapshot& values() const noexcept;
    [[nodiscard]] const UserPalette* palette() const noexcept;
private:
    std::chrono::microseconds elapsed_{};
    ValueSnapshot values_;
    const UserPalette* palette_ = nullptr;
};
} // namespace mg::graphics
