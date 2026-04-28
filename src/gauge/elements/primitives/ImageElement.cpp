#include <multigauge/gauge/elements/primitives/ImageElement.h>

bool ImageElement::init(AssetManager &assetManager, GraphicsContext& context) {
    if (imagePath.empty()) {
        image = Image();
        return true;
    }
    return assetManager.loadImage(context, imagePath, image);
}

void ImageElement::draw(Graphics &g) const {
    if (image.empty()) return;

    const auto& b = getBounds().toInt();

    g.drawImageArea(image, b);
}
