#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/image/Image.h>
#include <multigauge/properties/Codec.h>
#include <multigauge/properties/EnumTraits.h>

#include <cstdint>
#include <string>

namespace mg::gauge {

/// @brief Controls how an image is placed within the element layout bounds.
enum class ImageFitMode : std::uint8_t { Fit, Fill, Stretch };

/// @brief Draws an image asset element.
class ImageElement final : public Element {
    MG_EDITOR_NAME("Image")
    MG_TYPE_ID("image")

public:
    /// @brief Creates an image element.
    ImageElement() : Element(staticTypeId()) {}

    /// @brief Loads the image resource.
    bool init(std::string_view packageId, ::mg::AssetManager&, ::mg::graphics::GraphicsContext&) override;

    /// @brief Draws the loaded image in its layout bounds.
    void draw(::mg::graphics::Graphics&, const ::mg::Rect<float>&) const override;

private:
    std::string imagePath_;
    ImageFitMode fit_ = ImageFitMode::Fit;
    ::mg::images::Image image_;

    MG_PROPS_PARENT(Element)
    MG_PROPS_BEGIN()
    MG_PROP(imagePath_, "path", "Image Path", "Filepath of image.")
    MG_PROP(fit_, "fit", "Fit", "How the image is placed within its layout bounds.")
    MG_PROPS_END()
};

} // namespace mg::gauge

namespace mg {

template <> struct EnumTraits<gauge::ImageFitMode> {
    static constexpr EnumOption<gauge::ImageFitMode> options[] = {
        {gauge::ImageFitMode::Fit, "fit", "Fit"},
        {gauge::ImageFitMode::Fill, "fill", "Fill"},
        {gauge::ImageFitMode::Stretch, "stretch", "Stretch"},
    };
};

CODEC_BEGIN(gauge::ImageFitMode)
    DECODE() { return decodeEnum(v, out); }

    ENCODE() { return encodeEnum(out, v); }
CODEC_END()

} // namespace mg
