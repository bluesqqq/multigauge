#pragma once

#include <multigauge/graphics/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

class ImageElement : public Element {
    private:
        const char* path = "";

        Image image;

    public:
        explicit ImageElement(Element* parent);

        ImageElement(Element* parent, const rapidjson::Value::ConstObject json);

        bool init(AssetManager& assetManager) override;
        void draw(Graphics& g) const override;
};
