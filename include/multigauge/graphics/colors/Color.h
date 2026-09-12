#pragma once

#include <multigauge/properties/meta/Inspector.h>

#include <cstdint>
#include <memory>

#include <multigauge/graphics/colors/ColorResolver.h>
#include <multigauge/graphics/colors/rgba.h>
#include <multigauge/properties/PropertyObject.h>

#define DEFAULT_COLOR { 0, 0, 0, 255 }

namespace mg::graphics {

class Color;
using OwnedColor = std::unique_ptr<Color>;

/// Immutable-at-render-time color definition. The property system may mutate a
/// definition between frames; a ColorResolver snapshots every dynamic input for
/// the duration of one frame.
class Color : public ::mg::PropertyObject {
    MG_EDITOR_NAME("Color")

public:
    virtual ~Color() = default;
    MG_POLYMORPHIC_REGISTRY(OwnedColor)

    virtual OwnedColor clone() const = 0;

    Color(const Color&) noexcept;
    Color& operator=(const Color&) noexcept { return *this; }
    [[nodiscard]] std::uint32_t id() const noexcept { return id_; }

protected:
    Color() noexcept;
    /// Implementations must resolve nested definitions through `frame` and
    /// return transparent black for invalid/missing inputs.
    virtual rgba resolveUncached(const ColorResolver::Frame& frame) const noexcept = 0;

private:
    friend class ColorResolver;
    std::uint32_t id_;
};

struct Paint : public ::mg::PropertyObject {
    MG_EDITOR_NAME("Paint")

    OwnedColor fill;
    OwnedColor stroke;
    float thickness = 1.0f;

    MG_PROPS_BEGIN()
    MG_PROP(fill, "fill")
    MG_PROP(stroke, "stroke")
    MG_PROP(thickness, "thickness")
    MG_PROPS_END()

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Paint", {
        MG_PROPERTY("fill", "Fill", widget::color);
        MG_PROPERTY("stroke", "Stroke", widget::color);
        MG_PROPERTY("thickness", "Stroke Width", widget::number);
    });
    MG_INSPECTOR_END()
#endif

public:
    Paint();
    Paint(OwnedColor fill, OwnedColor stroke, float thickness = 1.0f);
};

} // namespace mg::graphics

namespace mg {

template <> struct PropertyNullableTraits<graphics::OwnedColor> { static constexpr bool value = true; };

template <> struct MgPolymorphicRegistryTraits<graphics::OwnedColor> {
    static constexpr bool supported = true;
    static bool getTypesMeta(json::Writer& writer) { return graphics::Color::registry().writeTypesMeta(writer); }
};

CODEC_BEGIN(graphics::OwnedColor)
    DECODE();
    ENCODE();
CODEC_END()

} // namespace mg
