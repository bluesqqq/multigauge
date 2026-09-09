#pragma once

#include <cstdint>
#include <optional>
#include <vector>

#include <multigauge/gauge/Element.h>
#include <multigauge/gauge/ticks/TickList.h>
#include <multigauge/graphics/colors/Color.h>
#include <multigauge/properties/EnumTraits.h>
#include <multigauge/value/ValueView.h>

namespace mg::gauge {

/// @brief The visual form rendered by a radial part.
enum class RadialPartType : std::uint8_t { Needle, Scale };

/// @brief A drawable component owned by a Radial element.
class RadialPart : public ::mg::PropertyObject {
public:
    RadialPartType type = RadialPartType::Needle;
    ::mg::graphics::Paint paint;
    ::mg::gauge::TickList ticks;
    float radius = 1.0F;

    MG_PROPS_BEGIN()
        MG_PROP(type, "type")
        MG_PROP(paint, "paint")
        MG_PROP(ticks, "ticks")
        MG_PROP(radius, "radius")
    MG_PROPS_END()
};

/// @brief Draws a value-driven circular gauge and its embedded visual parts.
class Radial final : public Element {
    MG_EDITOR_NAME("Radial")
    MG_TYPE_ID("radial")

public:
    /// @brief Creates a radial element.
    Radial() : Element(staticTypeId()) {}

    /// @brief Draws all parts using this element's common radial state.
    void draw(::mg::graphics::Graphics&, const ::mg::Rect<float>&) const override;

    /// @brief Advances transient state used by radial parts.
    void update(std::chrono::microseconds) override;

private:
    std::optional<::mg::ValueView> value_;
    float startAngle_ = 0.0F;
    float endAngle_ = 360.0F;
    std::vector<RadialPart> parts_;

    MG_PROPS_PARENT(Element)
    MG_PROPS_BEGIN()
        MG_PROP(value_, "value")
        MG_PROP(startAngle_, "startAngle")
        MG_PROP(endAngle_, "endAngle")
        MG_PROP(parts_, "parts")
    MG_PROPS_END()

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
        MG_SECTION("Radial", {
            MG_PROPERTY("value", "Value", "value-ref");
            MG_ROW({
                MG_PROPERTY("startAngle", "Start Angle", "number");
                MG_PROPERTY("endAngle", "End Angle", "number");
            });
        });
        MG_SECTION("Parts", {
            MG_PROPERTY("parts", "Parts", "radial-parts");
        });
        MG_INCLUDE("layout");
    MG_INSPECTOR_END()
#endif
};

} // namespace mg::gauge

namespace mg {

template <> struct EnumTraits<gauge::RadialPartType> {
    static constexpr EnumOption<gauge::RadialPartType> options[] = {
        {gauge::RadialPartType::Needle, "needle", "Needle"},
        {gauge::RadialPartType::Scale, "scale", "Scale"},
    };
};

CODEC_BEGIN(gauge::RadialPartType)
    DECODE() { return decodeEnum(v, out); }
    ENCODE() { return encodeEnum(out, v); }
CODEC_END()

} // namespace mg
