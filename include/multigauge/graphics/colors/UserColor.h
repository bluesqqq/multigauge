#pragma once

#include <multigauge/graphics/colors/Color.h>

namespace mg::graphics {

class UserColor final : public Color {
    MG_EDITOR_NAME("User Color")
    MG_TYPE_ID("user")

public:
    enum class Slot : std::uint8_t { Primary, Secondary, Background };

    explicit UserColor(Slot slot = Slot::Primary);
    OwnedColor clone() const override;

protected:
    rgba resolveUncached(const ColorResolver::Frame& frame) const noexcept override;

private:
    Slot slot;
    MG_PROPS_PARENT(Color)
    MG_PROPS_BEGIN()
    MG_PROP(slot, "slot", "Slot", "User palette slot.")
    MG_PROPS_END()
};

} // namespace mg::graphics

namespace mg {
template<> struct EnumTraits<graphics::UserColor::Slot> {
    static constexpr EnumOption<graphics::UserColor::Slot> options[] = {
        { graphics::UserColor::Slot::Primary, "primary", "Primary" },
        { graphics::UserColor::Slot::Secondary, "secondary", "Secondary" },
        { graphics::UserColor::Slot::Background, "background", "Background" },
    };
};

CODEC_BEGIN(graphics::UserColor::Slot)
    DECODE() { return decodeEnum(v, out); }
    ENCODE() { return encodeEnum(out, v); }
CODEC_END()
} // namespace mg
