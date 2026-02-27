#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

class ImageElement : public Element {
    private:
        const char* path = "";

        Image image;

        // Editable props list
        MG_EDITABLE_BEGIN()
            MG_PROP(path)
        MG_EDITABLE_END()

    public:
        explicit ImageElement(Element* parent);

        ImageElement(Element* parent, const rapidjson::Value::ConstObject json);

        bool init(AssetManager& assetManager) override;
        void draw(Graphics& g) const override;
};

REGISTER_ELEMENT_TYPE("image", ImageElement);