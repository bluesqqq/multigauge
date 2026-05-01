#pragma once

#include <multigauge/geometry/Rect.h>

#include <multigauge/graphics/Graphics.h>
#include <yoga/Yoga.h>

#include <rapidjson/document.h>

#include <multigauge/AssetManager.h>

#include <multigauge/properties/PropertyObject.h>

#include <multigauge/gauge/Layout.h>

#include <cstdint>
#include <memory>
#include <vector>

namespace mg::gauge {

using ::mg::AssetManager;
using ::mg::Rect;
using ::mg::graphics::Graphics;
using ::mg::graphics::GraphicsContext;

class Element;
using OwnedElement = std::unique_ptr<Element>;

class GaugeFace;

class Element : public ::mg::PropertyObject {
    MG_EDITOR_NAME("Element")

    private:
        //----------[ TREE ]----------//

        /// Parent element in the hierarchy, or `nullptr` if root.
        Element* parent = nullptr;
        /// Owned child elements.
        std::vector<OwnedElement> children;

        //----------[ LAYOUT ]----------//

        Layout style;
        /// Absolute bounds computer from Yoga.
        Rect<float> bounds = Rect<float>(0.0f, 0.0f, 0.0f, 0.0f);
        /// Yoga node for this element.
        YGNodeRef node = nullptr;

    protected:
        GaugeFace* face = nullptr;

        friend class GaugeFace;

        //----------[ LIFETIME HOOKS ]----------//

        virtual bool init(AssetManager& assetManager, GraphicsContext& context) { return true; }
        virtual void draw(Graphics& g) const {}
        virtual void update(int deltaTime) {}

    public:
        enum Type { Base, Circular };

        MG_POLYMORPHIC_REGISTRY_WITH_ARGS(OwnedElement, Element*)

        explicit Element(Element* parent = nullptr);
        virtual ~Element();

        virtual Type getType() const { return Type::Base; }
        
        //----------[ TREE ]----------//

        GaugeFace* getOwnerFace() const {
            const Element* current = this;
            while (current->parent) current = current->parent;
            return current->face;
        }
        Element* getParent() const { return parent; }

        //----------[ CHILDREN ]----------//

        std::size_t childCount() const { return children.size(); }
        Element* childAt(std::size_t i) { return children[i].get(); }
        const Element* childAt(std::size_t i) const { return children[i].get(); }

        bool insertChild(OwnedElement child, std::size_t index);
        OwnedElement removeChild(Element* child);

        //----------[ LIFETIME ]----------//

        bool initRecursive(AssetManager& assetManager, GraphicsContext& context);
        void drawRecursive(Graphics& g) const;
        void updateRecursive(int deltaTime);

        //----------[ LAYOUT ]----------//
        
        void layoutRecursive(float parentAbsX, float parentAbsY);
        YGNodeRef getNode() const { return node; }
        const Rect<float>& getBounds() const { return bounds; }

        //----------[ SERIALIZATION ]----------//

        static bool setChildren(::mg::PropertyObject* obj, const rapidjson::Value& v);
        static bool getChildren(const ::mg::PropertyObject* obj, rapidjson::Value& out, rapidjson::Document::AllocatorType& a);

        MG_PROPS_BEGIN()
            MG_PROP(style, "style", "Style", "Layout options.")
            MG_PROP_CUSTOM("children", "Children", "Child elements.", &Element::setChildren, &Element::getChildren)
        MG_PROPS_END()
};

} // namespace mg::gauge

namespace mg {

CODEC_BEGIN(gauge::OwnedElement)
    DECODE();

    ENCODE();
CODEC_END()

} // namespace mg
