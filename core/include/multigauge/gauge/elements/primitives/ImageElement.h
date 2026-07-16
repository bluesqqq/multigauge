#pragma once

#include <multigauge/gauge/Element.h>
#include <multigauge/graphics/colors/Color.h>

#include <string>

namespace mg::gauge {

using ::mg::images::Image;

class ImageElement : public Element {
        MG_EDITOR_NAME("Image")
    MG_TYPE_ID("image")
    private:
        std::string imagePath;

        Image image;

        // PropertyObject props list
        MG_PROPS_PARENT(Element)
        MG_PROPS_BEGIN()
    MG_PROP(imagePath, "path", "Image Path", "Filepath of image.")
        MG_PROPS_END()

    public:
        using Element::Element;
        
        bool init(AssetManager& assetManager, GraphicsContext& context) override;
        void draw(Graphics& g) const override;
};

} // namespace mg::gauge
