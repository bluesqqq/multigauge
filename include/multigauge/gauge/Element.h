#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <string_view>

#include <multigauge/container/GenerationalHandle.h>
#include <multigauge/gauge/layout/ChildAlignment.h>
#include <multigauge/gauge/layout/Direction.h>
#include <multigauge/gauge/layout/Floating.h>
#include <multigauge/gauge/layout/Padding.h>
#include <multigauge/gauge/layout/Size.h>
#include <multigauge/graphics/geometry/Rect.h>
#include <multigauge/properties/PolymorphicRegistry.h>
#include <multigauge/properties/PropertyObject.h>
#if MG_BUILD_EDITOR
#include <multigauge/properties/meta/Inspector.h>
#endif

namespace mg {

// Forward Declarations
namespace graphics { class Graphics; class GraphicsContext; }
class AssetManager;

namespace gauge {

class GaugeFace; // Forward Declaration

/// @brief Stable reference to an element owned by a gauge face.
using NodeHandle = ::mg::GenerationalHandle<struct NodeTag>;

/// @brief Represents a single gauge element.
class Element : public ::mg::PropertyObject {
public:
    /// @brief Layout state available to elements in the face tree.
    struct Layout : ::mg::PropertyObject {
        layout::Size width;
        layout::Size height;
        layout::Direction direction = layout::Direction::Vertical;
        layout::Padding padding;
        int childGap = 0;
        layout::ChildAlignment childAlignment;
        layout::Floating floating;
        float aspectRatio = 0.0F;

        MG_PROPS_BEGIN()
            MG_PROP(width, "width")
            MG_PROP(height, "height")
            MG_PROP(direction, "direction")
            MG_PROP(padding, "padding")
            MG_PROP(childGap, "childGap")
            MG_PROP(childAlignment, "childAlignment")
            MG_PROP(floating, "floating")
            MG_PROP(aspectRatio, "aspectRatio")
        MG_PROPS_END()

#if MG_BUILD_EDITOR
        MG_INSPECTOR_BEGIN()
        MG_SECTION("Size", {
            MG_LABELED_ROW("Dimensions", {
                MG_PROPERTY("width", "Width", widget::layoutSize);
                MG_PROPERTY("height", "Height", widget::layoutSize);
            });
            MG_PROPERTY("aspectRatio", "Aspect Ratio", widget::number);
        });
        MG_SECTION("Layout", {
            MG_CONTROL(widget::directionToggle, {MG_BIND("value", "direction")});
            MG_ROW({
                MG_CONTROL(widget::alignmentGrid, {MG_BIND("value", "childAlignment")});
                MG_PROPERTY("childGap", "Child Gap", widget::number);
            });
            MG_CONTROL(widget::insets, {MG_BIND("value", "padding")});
        });
        MG_SECTION("Position", {
            MG_PROPERTY("floating.mode", "Mode", widget::select);
            MG_CONTROL_IF(widget::anchorPair, MG_IN("floating.mode", "relative", "absolute"),
                          {MG_BIND("target", "floating.parentAnchor"),
                           MG_BIND("element", "floating.elementAnchor")});
            MG_CONTROL_IF(widget::axisPair, MG_IN("floating.mode", "relative", "absolute"),
                          {MG_BIND("value", "floating.offset")});
            MG_PROPERTY_IF("floating.zIndex", "Z Index", widget::number, MG_IN("floating.mode", "relative", "absolute"));
            MG_CONTROL_IF(widget::axisPair, MG_IN("floating.mode", "relative", "absolute"),
                          {MG_BIND("value", "floating.expand")});
        });
        MG_INSPECTOR_END()
#endif
    };

    using OwnedElement = std::unique_ptr<Element>;
    MG_POLYMORPHIC_REGISTRY(OwnedElement)

    /// @brief Returns the stable identifier written by the polymorphic codec.
    [[nodiscard]] const char* typeId() const override { return typeId_.data(); }

    //----------[ CTOR + DTOR ]----------//

    /// @brief Creates an element with a stable, static type identifier.
    /// @param typeId Serialized type identifier. Its storage must outlive this element.
    explicit constexpr Element(std::string_view typeId = {}) noexcept : typeId_(typeId) {}

    /// @brief Destroys this type-specific element state.
    virtual ~Element() = default;

    //----------[ LIFECYCLE ]----------//

    /// @brief Advances transient element state.
    /// @param delta Elapsed time since the preceding update call.
    virtual void update(std::chrono::microseconds delta) { (void)delta; }

    /// @brief Draws the element into the active graphics target.
    /// @param graphics Graphics command surface for the current frame.
    /// @param bounds Current absolute layout rectangle for this element.
    virtual void draw(
        ::mg::graphics::Graphics& graphics,
        const ::mg::Rect<float>& bounds
    ) const {
        (void)graphics;
        (void)bounds;
    }

    /// @brief Loads any external resources required before drawing.
    /// @param packageId Installed package ID, or empty for raw/editor assets.
    /// @param assetManager Asset provider for element resources.
    /// @param context Graphics backend context that owns loaded resources.
    /// @return True when initialization succeeds.
    virtual bool init(
        std::string_view,
        ::mg::AssetManager&,
        ::mg::graphics::GraphicsContext&
    ) { return true; }

private:
    friend class GaugeFace;

    //----------[ LAYOUT ]----------//

    /// @brief Returns mutable layout property state for face layout declaration.
    [[nodiscard]] Layout& layout() noexcept { return layout_; }

    /// @brief Returns layout property state for read-only face layout declaration.
    [[nodiscard]] const Layout& layout() const noexcept { return layout_; }

private:
    std::string_view typeId_; ///< Stable registry string for built-in types.
    Layout layout_;           ///< Layout configuration.

    MG_PROPS_BEGIN()
        MG_PROP(layout_, "layout")
    MG_PROPS_END()

#if MG_BUILD_EDITOR
    MG_INSPECTOR_BEGIN()
    MG_INCLUDE("layout");
    MG_INSPECTOR_END()
#endif
};

} // namespace gauge

CODEC_BEGIN(gauge::Element::OwnedElement)
    DECODE();
    ENCODE();
CODEC_END()

} // namespace mg
