#pragma once

#include <multigauge/graphics/colors/Color.h>

namespace mg::graphics {

class StaticColor final : public Color {
    MG_EDITOR_NAME("Static Color")
    MG_TYPE_ID("static")

    rgba color;
    MG_PROPS_PARENT(Color)
    MG_PROPS_BEGIN()
    MG_PROP(color, "color", "Color", "RGBA color value.")
    MG_PROPS_END()

public:
    StaticColor();
    explicit StaticColor(rgba color);
    OwnedColor clone() const override;
    [[nodiscard]] rgba value() const noexcept { return color; }

protected:
    rgba resolveUncached(const ColorResolver::Frame&) const noexcept override;
};

} // namespace mg::graphics
