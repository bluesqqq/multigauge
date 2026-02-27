#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

class ImageElement : public Element {
    private:
        const char* path = "";

        Image image;

        // Editable props list
        MG_EDITABLE_BEGIN()
            MG_EDITABLE_PROP(path)
        MG_EDITABLE_END()

    public:
        using Element::Element;
        
        bool init(AssetManager& assetManager) override;
        void draw(Graphics& g) const override;
};