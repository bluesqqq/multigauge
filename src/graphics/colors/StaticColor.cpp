#include "StaticColor.h"

StaticColor::StaticColor() : color(DEFAULT_COLOR) { }

StaticColor::StaticColor(rgba color) : color(color) {}

std::unique_ptr<Color> StaticColor::blended(rgba color, float alpha) const { return std::make_unique<StaticColor>(this->color.blended(color, alpha)); }

std::unique_ptr<Color> StaticColor::blended(const Color &other, float alpha) const { return other.blended(this->color, alpha); }

std::unique_ptr<Color> StaticColor::clone() const { return std::make_unique<StaticColor>(*this); }

rgba StaticColor::getColor() const { return color; }

Color::Type StaticColor::getType() const { return Type::Static; }
