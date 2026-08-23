#include <cmath>
#include <multigauge/gauge/elements/primitives/ImageElement.h>
#include <multigauge/graphics/Graphics.h>
#include <multigauge/runtime/AssetManager.h>

namespace mg::gauge {
namespace {

::mg::graphics::Graphics::ImageFit graphicsFit(ImageFitMode fit) {
    switch (fit) {
    case ImageFitMode::Fill: return ::mg::graphics::Graphics::ImageFit::Fill;
    case ImageFitMode::Stretch: return ::mg::graphics::Graphics::ImageFit::Stretch;
    case ImageFitMode::Fit: return ::mg::graphics::Graphics::ImageFit::Fit;
    }
    return ::mg::graphics::Graphics::ImageFit::Fit;
}

} // namespace

bool ImageElement::init(std::string_view packageId, ::mg::AssetManager& assets, ::mg::graphics::GraphicsContext& context) {
    if (imagePath_.empty()) {
        image_ = {};
        return true;
    }
    return assets.loadImage(context, packageId, imagePath_, image_);
}

void ImageElement::draw(::mg::graphics::Graphics& g, const ::mg::Rect<float>& bounds) const {
    if (!image_.empty()) g.drawImageArea(image_, bounds.toInt(), graphicsFit(fit_));
}
} // namespace mg::gauge
