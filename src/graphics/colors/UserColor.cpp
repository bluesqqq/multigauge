#include <multigauge/graphics/colors/UserColor.h>
#include <multigauge/graphics/colors/StaticColor.h>

rgba UserColor::userColors[3] = {
    rgba("white"),     // Primary
    rgba("red"),       // Secondary
    rgba("black")      // Background
};

UserColor::UserColor(Slot slot) : slot(slot) { }

OwnedColor UserColor::clone() const { return std::make_unique<UserColor>(*this); }

rgba UserColor::getColor() const { return userColors[static_cast<size_t>(slot)]; }

Color::Type UserColor::getType() const { return Type::User; }

OwnedColor UserColor::blended(rgba color, float alpha) const { return std::make_unique<StaticColor>(getColor().blended(color, alpha)); }

OwnedColor UserColor::blended(const Color &other, float alpha) const { return other.blended(getColor(), alpha); }