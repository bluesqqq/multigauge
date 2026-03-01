#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

class ImageElement : public Element {
    MG_EDITOR_NAME("Image")
    private:
        const char* path = "";

        Image image;

        // Editable props list
        MG_EDITABLE_BEGIN()
            MG_EDITABLE_PROP_META(path, "path", "Image Path", "Filepath of image.")
        MG_EDITABLE_END()

    public:
        using Element::Element;
        
        bool init(AssetManager& assetManager) override;
        void draw(Graphics& g) const override;
};