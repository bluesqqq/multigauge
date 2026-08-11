#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/rgba.h>

namespace mg::gauge {

/// @brief Draws a solid-color rectangular frame.
class FrameElement final : public Element {
    MG_EDITOR_NAME("Frame")
    MG_TYPE_ID("frame")

  public:
    /// @brief Creates a frame with the specified fill color.
    explicit FrameElement(::mg::graphics::rgba fill = {0, 0, 0, 255}) noexcept
        : Element(staticTypeId()), fill_(fill) {}

    /// @brief Returns the frame fill color.
    [[nodiscard]] ::mg::graphics::rgba fill() const noexcept { return fill_; }

    /// @brief Sets the frame fill color.
    void setFill(::mg::graphics::rgba value) noexcept { fill_ = value; }

    /// @brief Draws the frame in its layout bounds.
    void draw(::mg::graphics::Graphics& graphics, const ::mg::Rect<float>& bounds) const override;

private:
    ::mg::graphics::rgba fill_;

    MG_PROPS_PARENT(Element)
    MG_PROPS_BEGIN()
        MG_PROP(fill_, "fill", "Fill", "Frame fill color.")
    MG_PROPS_END()
};

} // namespace mg::gauge
