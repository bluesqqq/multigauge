#pragma once

#include <memory>
#include <chrono>
#include <type_traits>
#include <multigauge/graphics/colors/Color.h>
#include <multigauge/gauge/Element.h>
#include <vector>

namespace mg::gauge {

class GaugeFace : public ::mg::PropertyObject {
    MG_EDITOR_NAME("Gauge Face")

    private:
        friend class Element;
        YGNodeRef node = nullptr;

        std::vector<OwnedElement> children;

        OwnedColor backgroundColor;

        RootLayout style;
        bool rootLayoutDirty = true;
        bool layoutDirty = true;
        float layoutWidth = -1.0f;
        float layoutHeight = -1.0f;

        static bool setChildren(::mg::PropertyObject* obj, json::Reader value);
        static bool getChildren(const ::mg::PropertyObject* obj, json::Writer& writer);
        void markLayoutSubtreeDirty();

        MG_PROPS_BEGIN()
            MG_PROP_CALLBACK(style, "layout", "Layout", "Root layout options.", &GaugeFace::markLayoutDirty)
            MG_PROP(backgroundColor, "bgColor", "Background Color", "Background color.")
            MG_PROP_CUSTOM_HIDDEN("children", "Children", "Child elements.", &GaugeFace::setChildren, &GaugeFace::getChildren)
        MG_PROPS_END()

    public:
        GaugeFace();
        ~GaugeFace();

        /// Loads a gauge face document.
        /// @see docs/schemas/gauge.schema.json for the serialized shape.
        bool load(json::Reader value);

        /// Saves a gauge face document.
        /// @see docs/schemas/gauge.schema.json for the serialized shape.
        bool save(json::Writer& writer) const;

        //----------[ LIFETIME ]----------//

        bool init(AssetManager& assetManager, GraphicsContext& context);
        void update(std::chrono::microseconds delta);
        void draw(Graphics& g) const;

        //----------[ CHILDREN ]----------//

        std::size_t childCount() const { return children.size(); }
        Element* childAt(std::size_t i) { return children[i].get(); }
        const Element* childAt(std::size_t i) const { return children[i].get(); }

        bool insertChild(OwnedElement child, std::size_t index);
        OwnedElement removeChild(Element* child);

        //----------[ LAYOUT ]----------//

        void layout(Graphics& g);
        /// Invalidates root layout after a root-style or tree change.
        void markLayoutDirty();
        YGNodeRef getNode() const { return node; }
    };

} // namespace mg::gauge
