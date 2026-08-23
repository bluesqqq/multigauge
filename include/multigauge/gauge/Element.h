#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <string_view>

#include <multigauge/container/GenerationalHandle.h>
#include <multigauge/gauge/Layout.h>
#include <multigauge/graphics/geometry/Rect.h>
#include <multigauge/properties/PolymorphicRegistry.h>
#include <multigauge/properties/PropertyObject.h>

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
    [[nodiscard]] layout::Layout& layout() noexcept { return layout_; }

    /// @brief Returns layout property state for read-only face layout declaration.
    [[nodiscard]] const layout::Layout& layout() const noexcept { return layout_; }

private:
    std::string_view typeId_; ///< Stable registry string for built-in types.
    layout::Layout layout_;   ///< Layout configuration.

    MG_PROPS_BEGIN()
        MG_PROP(layout_, "layout", "Layout", "Layout options.")
    MG_PROPS_END()
};

} // namespace gauge

CODEC_BEGIN(gauge::Element::OwnedElement)
    DECODE();
    ENCODE();
CODEC_END()

} // namespace mg
