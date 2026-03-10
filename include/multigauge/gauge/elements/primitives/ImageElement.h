#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

class ImageElement : public Element {
    MG_EDITOR_NAME("Image")
    private:
        const char* path = "";

        Image image;

        // PropertyObject props list
        MG_PROPS_PARENT(Element)
        MG_PROPS_BEGIN()
            MG_PROP(path, "path", "Image Path", "Filepath of image.", "string")
        MG_PROPS_END()

    public:
        using Element::Element;
        
        bool init(AssetManager& assetManager) override;
        void draw(Graphics& g) const override;
};

