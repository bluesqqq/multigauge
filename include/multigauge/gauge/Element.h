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

class Element;
using OwnedElement = std::unique_ptr<Element>;

class Element : public PropertyObject {
    MG_EDITOR_NAME("Element")

    private:
        //----------[ TREE ]----------//

        /// @brief Parent element in the hierarchy, or nullptr if root
        Element* parent = nullptr;
        /// @brief Owned child elements
        std::vector<OwnedElement> children;

        //----------[ LAYOUT ]----------//

        Layout style;

        /// @brief Absolute bounds computer from Yoga
        Rect<float> bounds = Rect<float>(0.0f, 0.0f, 0.0f, 0.0f);
        /// @brief Yoga node for this element
        YGNodeRef node = nullptr;
        /// @brief Shared Yoga config
        YGConfigRef config = nullptr;
        /// @brief True when the layout needs to be recalculated at the root
        bool layoutDirty = true;

        void clearLayoutDirtyRecursive();

        void makeNode();
        void removeNode();

    protected:
        void markLayoutDirty();
        bool needsLayout() const { return layoutDirty; }

        //----------[ HOOKS ]----------//

        virtual bool init(AssetManager& assetManager) { return true; }
        virtual void draw(Graphics& g) const {}
        virtual void update(int deltaTime) {}

    public:
        enum Type { Base, Circular };

        MG_POLYMORPHIC_REGISTRY_WITH_ARGS(OwnedElement, Element*)

        explicit Element(Element* parent, YGConfigRef config = nullptr);
        virtual ~Element();

        virtual Type getType() const { return Type::Base; }
        
        //----------[ TREE ]----------//

        Element* getRoot() {
            Element* n = this;
            while (n->getParent()) n = n->getParent();
            return n;
        }
        Element* getParent() const { return parent; }
        YGNodeRef getNode() const { return node; }
        YGConfigRef getConfig() const { return config; }
        const Rect<float>& getBounds() const { return bounds; }

        bool isRoot() const { return parent == nullptr; }

        //----------[ CHILDREN ]----------//

        std::size_t childCount() const { return children.size(); }
        Element* childAt(std::size_t i) { return children[i].get(); }
        const Element* childAt(std::size_t i) const { return children[i].get(); }

        Element* addChild(const rapidjson::Value::ConstObject json);
        Element* insertChild(const rapidjson::Value::ConstObject json, std::size_t index);
        bool insertChild(OwnedElement child, std::size_t index);
        OwnedElement detachChild(Element* child);
        bool removeChild(Element* child);

        //----------[ TRAVERSAL ]----------//

        bool initRecursive(AssetManager& assetManager);
        void drawRecursive(Graphics& g) const;
        void updateRecursive(int deltaTime);

        //----------[ LAYOUT ]----------//

        void layoutRecursive(float width, float height, YGDirection direction = YGDirectionLTR);

        //----------[ LAYOUT ]----------//
        
        static OwnedElement fromJson(Element *parent, const rapidjson::Value::ConstObject json);
        void saveToJson(rapidjson::Value& out, rapidjson::Document::AllocatorType& a) const;

        void loadFromJson(const rapidjson::Value::ConstObject& json) {
            loadLayout(json);
            loadProps(json);
            loadChildren(json);
        }

        void loadLayout(const rapidjson::Value::ConstObject& json);

        void loadProps(const rapidjson::Value::ConstObject& json);

        void loadChildren(const rapidjson::Value::ConstObject& json);

        MG_PROPS_BEGIN()
            MG_PROP(style, "style", "Style", "Layout options.")
        MG_PROPS_END()
};
