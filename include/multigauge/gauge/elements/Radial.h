#pragma once

#include <memory>
#include <optional>
#include <vector>

#include <multigauge/gauge/Element.h>
#include <multigauge/gauge/ticks/TickList.h>
#include <multigauge/graphics/colors/Color.h>
#include <multigauge/value/ValueView.h>

namespace mg::gauge {

/// @brief A drawable component owned by a Radial element.
class RadialPart : public ::mg::PropertyObject {
public:
    using OwnedPart = std::unique_ptr<RadialPart>;
    MG_POLYMORPHIC_REGISTRY(OwnedPart)
    virtual ~RadialPart() = default;

    /// @brief Draws this part with the radial's shared value and geometry.
    virtual void draw(::mg::graphics::Graphics&, ::mg::Point<float>, float,
                      const ::mg::ValueView&, float, float) const = 0;
    /// @brief Updates transient state from the radial's shared value.
    virtual void update(const ::mg::ValueView&) {}
};

/// @brief Draws a value-driven radial needle.
class NeedlePart final : public RadialPart {
    MG_EDITOR_NAME("Needle")
    MG_TYPE_ID("needle")
public:
    void draw(::mg::graphics::Graphics&, ::mg::Point<float>, float,
              const ::mg::ValueView&, float, float) const override;
private:
    ::mg::graphics::Paint paint_;
    float radius_ = 1.0F;
    MG_PROPS_BEGIN()
        MG_PROP(paint_, "paint")
        MG_PROP(radius_, "radius")
    MG_PROPS_END()
#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Needle", {
        MG_PROPERTY("paint.fill", "Fill", "color");
        MG_PROPERTY("paint.stroke", "Stroke", "color");
        MG_PROPERTY("paint.thickness", "Stroke Width", "number");
        MG_PROPERTY("radius", "Radius", "number");
    });
    MG_INSPECTOR_END()
#endif
};

/// @brief Draws a circular tick scale.
class ScalePart final : public RadialPart {
    MG_EDITOR_NAME("Scale")
    MG_TYPE_ID("scale")
public:
    void draw(::mg::graphics::Graphics&, ::mg::Point<float>, float,
              const ::mg::ValueView&, float, float) const override;
    void update(const ::mg::ValueView&) override;
private:
    ::mg::gauge::TickList ticks_;
    float radius_ = 1.0F;
    MG_PROPS_BEGIN()
        MG_PROP(ticks_, "ticks")
        MG_PROP(radius_, "radius")
    MG_PROPS_END()
#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_SECTION("Scale", { MG_PROPERTY("radius", "Radius", "number"); });
    MG_INCLUDE("ticks");
    MG_INSPECTOR_END()
#endif
};

/// @brief Draws a value-driven circular gauge and its embedded visual parts.
class Radial final : public Element {
    MG_EDITOR_NAME("Radial")
    MG_TYPE_ID("radial")
public:
    Radial() : Element(staticTypeId()) {}
    void draw(::mg::graphics::Graphics&, const ::mg::Rect<float>&) const override;
    void update(std::chrono::microseconds) override;
private:
    std::optional<::mg::ValueView> value_;
    float startAngle_ = 0.0F;
    float endAngle_ = 360.0F;
    std::vector<RadialPart::OwnedPart> parts_;
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
        MG_ROW({ MG_PROPERTY("startAngle", "Start Angle", "number"); MG_PROPERTY("endAngle", "End Angle", "number"); });
    });
    MG_SECTION("Parts", { MG_PROPERTY("parts", "Parts", "radial-parts"); });
    MG_INCLUDE("layout");
    MG_INSPECTOR_END()
#endif
};

} // namespace mg::gauge

namespace mg {
template <> struct MgPolymorphicRegistryTraits<gauge::RadialPart::OwnedPart> {
    static constexpr bool supported = true;
    static bool getTypesMeta(json::Writer& writer) { return gauge::RadialPart::registry().writeTypesMeta(writer); }
};
CODEC_BEGIN(gauge::RadialPart::OwnedPart)
    DECODE();
    ENCODE();
CODEC_END()
} // namespace mg
