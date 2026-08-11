#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/image/Image.h>
#include <string>

namespace mg::gauge {

/// @brief Draws an image asset element.
class ImageElement final : public Element {
    MG_EDITOR_NAME("Image")
    MG_TYPE_ID("image")

public:
    /// @brief Creates an image element.
    ImageElement() : Element(staticTypeId()) {}

    /// @brief Loads the image resource.
    bool init(::mg::AssetManager&, ::mg::graphics::GraphicsContext&) override;

    /// @brief Draws the loaded image in its layout bounds.
    void draw(::mg::graphics::Graphics&, const ::mg::Rect<float>&) const override;

private:
    std::string imagePath_;
    ::mg::images::Image image_;

    MG_PROPS_PARENT(Element)
    MG_PROPS_BEGIN()
    MG_PROP(imagePath_, "path", "Image Path", "Filepath of image.")
    MG_PROPS_END()
};

} // namespace mg::gauge
