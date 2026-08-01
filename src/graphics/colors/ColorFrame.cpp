#include <multigauge/graphics/colors/ColorFrame.h>

namespace mg::graphics {
void ValueSnapshot::refresh() noexcept {
    ValueRegistry::forEachBuiltIn([this](ValueHandle handle) { builtIns_[handle.builtInId()] = ValueRegistry::value(handle); });
    for (std::size_t slot = 0; slot < users_.size(); ++slot) { users_[slot] = 0.0F; generations_[slot] = 0; }
    ValueRegistry::forEachUser([this](ValueHandle handle) { users_[handle.userSlot()] = ValueRegistry::value(handle); generations_[handle.userSlot()] = handle.userGeneration(); });
}
Measurement ValueSnapshot::value(ValueHandle handle, Measurement fallback) const noexcept {
    if (handle.isBuiltIn() && handle.builtInId() < builtIns_.size()) return builtIns_[handle.builtInId()];
    if (handle.isUser() && handle.userSlot() < users_.size() && generations_[handle.userSlot()] == handle.userGeneration()) return users_[handle.userSlot()];
    return fallback;
}
void ColorFrame::refresh(std::chrono::microseconds elapsed, const UserPalette& palette) noexcept { elapsed_ = elapsed; palette_ = &palette; values_.refresh(); }
std::chrono::microseconds ColorFrame::elapsed() const noexcept { return elapsed_; }
const ValueSnapshot& ColorFrame::values() const noexcept { return values_; }
const UserPalette* ColorFrame::palette() const noexcept { return palette_; }
} // namespace mg::graphics
