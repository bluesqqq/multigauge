#include <multigauge/graphics/colors/ColorFrame.h>

#include <span>
#include <cstdint>

namespace mg::graphics {

void ValueSnapshot::refresh() noexcept {
    const std::span<const ::mg::Value> source = ::mg::Value::list();
    first_ = source.empty() ? nullptr : source.data();
    values_.resize(source.size());
    for (std::size_t index = 0; index < source.size(); ++index) values_[index] = source[index].valueBase();
}

float ValueSnapshot::value(const ::mg::Value* value, float fallback) const noexcept {
    if (!first_ || !value) return fallback;
    const std::uintptr_t first = reinterpret_cast<std::uintptr_t>(first_);
    const std::uintptr_t address = reinterpret_cast<std::uintptr_t>(value);
    const std::size_t bytes = values_.size() * sizeof(::mg::Value);
    if (address < first || address - first >= bytes || (address - first) % sizeof(::mg::Value) != 0) return fallback;
    return values_[(address - first) / sizeof(::mg::Value)];
}

void ColorFrame::refresh(std::chrono::microseconds elapsed, const UserPalette& palette) noexcept {
    elapsed_ = elapsed;
    palette_ = &palette;
    values_.refresh();
}

} // namespace mg::graphics
