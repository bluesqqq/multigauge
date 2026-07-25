#pragma once

#include <memory>
#include <type_traits>
#include <multigauge/graphics/colors/Color.h>
#include <multigauge/gauge/Element.h>
#include <vector>

namespace mg::gauge {

class GaugeFace : public ::mg::PropertyObject {
    MG_EDITOR_NAME("Gauge Face")

    private:
        YGNodeRef node = nullptr;

        std::vector<OwnedElement> children;

        OwnedColor backgroundColor;

        RootLayout style;

        static bool setChildren(::mg::PropertyObject* obj, json::Reader value);
        static bool getChildren(const ::mg::PropertyObject* obj, json::Writer& writer);

        MG_PROPS_BEGIN()
            MG_PROP(style, "layout", "Layout", "Root layout options.")
            MG_PROP(backgroundColor, "bgColor", "Background Color", "Background color.")
            MG_PROP_CUSTOM_HIDDEN("children", "Children", "Child elements.", &GaugeFace::setChildren, &GaugeFace::getChildren)
        MG_PROPS_END()

    public:
        GaugeFace();
        ~GaugeFace();

        bool load(json::Reader value);
        bool save(json::Writer& writer) const;

        //----------[ LIFETIME ]----------//

        bool init(AssetManager& assetManager, GraphicsContext& context);
        void update(int deltaTime);
        void draw(Graphics& g) const;

        //----------[ CHILDREN ]----------//

        std::size_t childCount() const { return children.size(); }
        Element* childAt(std::size_t i) { return children[i].get(); }
        const Element* childAt(std::size_t i) const { return children[i].get(); }

        bool insertChild(OwnedElement child, std::size_t index);
        OwnedElement removeChild(Element* child);

        //----------[ LAYOUT ]----------//

        void layout(Graphics& g);
        YGNodeRef getNode() const { return node; }
    };

} // namespace mg::gauge
