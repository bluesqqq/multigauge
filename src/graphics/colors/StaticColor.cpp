#include <multigauge/graphics/colors/StaticColor.h>

namespace mg::graphics {

StaticColor::StaticColor() : color(DEFAULT_COLOR) {}
StaticColor::StaticColor(rgba color) : color(color) {}
OwnedColor StaticColor::clone() const { return std::make_unique<StaticColor>(*this); }
rgba StaticColor::resolveUncached(const ColorResolver::Frame&) const noexcept { return color; }

} // namespace mg::graphics
