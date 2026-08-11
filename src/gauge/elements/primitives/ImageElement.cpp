#include <cmath>
#include <multigauge/gauge/elements/primitives/ImageElement.h>
#include <multigauge/graphics/Graphics.h>
#include <multigauge/runtime/AssetManager.h>

namespace mg::gauge {
bool ImageElement::init(::mg::AssetManager& assets, ::mg::graphics::GraphicsContext& context) {
    if (imagePath_.empty()) {
        image_ = {};
        return true;
    }
    return assets.loadImage(context, imagePath_, image_);
}

void ImageElement::draw(::mg::graphics::Graphics& g, const ::mg::Rect<float>& bounds) const {
    if (!image_.empty()) g.drawImageArea(image_, bounds.toInt());
}
} // namespace mg::gauge
