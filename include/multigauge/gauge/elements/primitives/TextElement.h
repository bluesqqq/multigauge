#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/TextPaint.h>
#include <multigauge/graphics/geometry/alignment.h>
#include <string>

namespace mg::gauge {

/// @brief Draws text content in a layout rectangle.
class TextElement final : public Element {
    MG_EDITOR_NAME("Text")
    MG_TYPE_ID("text")

public:
    /// @brief Creates a text element.
    TextElement() : Element(staticTypeId()) {}

    /// @brief Draws text in its layout bounds.
    void draw(::mg::graphics::Graphics&, const ::mg::Rect<float>&) const override;

private:
    std::string text_;
    ::mg::graphics::TextPaint paint_;
    ::mg::Anchor anchor_ = ::mg::Anchor::TopLeft;
    bool useEllipses_ = false;
    bool useHyphens_ = false;

    MG_PROPS_PARENT(Element)
    MG_PROPS_BEGIN()
    MG_PROP(text_, "text")
    MG_PROP(paint_, "paint")
    MG_PROP(useEllipses_, "ellipses")
    MG_PROP(useHyphens_, "hyphens")
    MG_PROPS_END()
};

} // namespace mg::gauge
