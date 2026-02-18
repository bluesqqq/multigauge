#include "UserColor.h"
#include "StaticColor.h"

UserColor::UserColor(Slot slot) : slot(slot) { }

UserColor::UserColor(const rapidjson::Value::ConstObject json) {
    if (json.HasMember("Slot") && json["slot"].IsString()) {
        const char* slotStr = json["slot"].GetString();

        if (slotStr == "primary")       slot = Slot::Primary;
        else if (slotStr == "secondary") slot = Slot::Secondary;
        else                             slot = Slot::Background;
    } else {
        slot = Slot::Background;
    }
}

std::unique_ptr<Color> UserColor::clone() const { return std::make_unique<UserColor>(*this); }

rgba UserColor::getColor() const { return userColors[static_cast<size_t>(slot)]; }

Color::Type UserColor::getType() const { return Type::User; }

std::unique_ptr<Color> UserColor::blended(rgba color, float alpha) const { return std::make_unique<StaticColor>(getColor().blended(color, alpha)); }

std::unique_ptr<Color> UserColor::blended(const Color &other, float alpha) const { return other.blended(getColor(), alpha); }