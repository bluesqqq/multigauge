#include "ImageElement.h"

ImageElement::ImageElement(Element* parent) : Element(parent), path("/placeholder.bmp") { }

ImageElement::ImageElement(Element* parent, const rapidjson::Value::ConstObject json) : Element(parent, json) {
    if (!json.HasMember("props") || !json["props"].IsObject()) return;
    const rapidjson::Value::ConstObject props = json["props"].GetObject();

    setCString(props, "path", path);
}

bool ImageElement::init(AssetManager &assetManager) {
    return assetManager.loadImage(path, image);
}

void ImageElement::draw(Graphics &g) const {
    const auto& b = getBounds().toInt();

    g.drawImage(image, b);
}

REGISTER_ELEMENT_TYPE("image", ImageElement);