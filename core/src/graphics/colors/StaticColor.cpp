#include <multigauge/graphics/colors/StaticColor.h>

namespace mg::graphics {

StaticColor::StaticColor() : color(DEFAULT_COLOR) { }

StaticColor::StaticColor(rgba color) : color(color) {}

OwnedColor StaticColor::blended(rgba color, float alpha) const { return std::make_unique<StaticColor>(this->color.blended(color, alpha)); }

OwnedColor StaticColor::blended(const Color &other, float alpha) const { return other.blended(this->color, alpha); }

OwnedColor StaticColor::clone() const { return std::make_unique<StaticColor>(*this); }

rgba StaticColor::getColor() const { return color; }

Color::Type StaticColor::getType() const { return Type::Static; }

} // namespace mg::graphics
