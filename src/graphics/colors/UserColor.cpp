#include <multigauge/graphics/colors/UserColor.h>

namespace mg::graphics {

UserColor::UserColor(Slot slot) : slot(slot) {}
OwnedColor UserColor::clone() const { return std::make_unique<UserColor>(*this); }

rgba UserColor::resolveUncached(const ColorResolver::Frame& frame) const noexcept {
    const ColorFrame* data = frame.data();
    const UserPalette* palette = data ? data->palette() : nullptr;
    return palette ? palette->color(static_cast<std::size_t>(slot)) : rgba{0, 0, 0, 0};
}

} // namespace mg::graphics
