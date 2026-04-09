#include <multigauge/gauge/elements/primitives/ImageElement.h>

bool ImageElement::init(AssetManager &assetManager) {
    if (path.empty()) {
        image = Image();
        return true;
    }
    return assetManager.loadImage(path, image);
}

void ImageElement::draw(Graphics &g) const {
    if (image.empty()) return;

    const auto& b = getBounds().toInt();

    g.drawImageArea(image, b);
}
