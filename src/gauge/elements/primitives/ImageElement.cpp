#include <multigauge/gauge/elements/primitives/ImageElement.h>

ImageElement::ImageElement(Element* parent) : Element(parent), path("/placeholder.bmp") { }

bool ImageElement::init(AssetManager &assetManager) {
    return assetManager.loadImage(path, image);
}

void ImageElement::draw(Graphics &g) const {
    const auto& b = getBounds().toInt();

    g.drawImageArea(image, b);
}