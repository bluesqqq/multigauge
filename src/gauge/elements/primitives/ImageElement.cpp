#include <multigauge/gauge/elements/primitives/ImageElement.h>

bool ImageElement::init(AssetManager &assetManager) {
    return assetManager.loadImage(path ? path : "", image);
}

void ImageElement::draw(Graphics &g) const {
    const auto& b = getBounds().toInt();

    g.drawImageArea(image, b);
}
