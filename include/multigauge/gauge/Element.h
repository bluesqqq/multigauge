#pragma once

#include <multigauge/geometry/Rect.h>

#include <multigauge/graphics/Graphics.h>
#include <yoga/Yoga.h>

#include <rapidjson/document.h>

#include <multigauge/AssetManager.h>
#include <multigauge/json/rj_helpers.h>

#include <multigauge/editor/Editable.h>

#include <memory>

class Element;
using OwnedElement = std::unique_ptr<Element>;

class Element : public Editable {
    private:
        //----------[ TREE ]----------//

        /// @brief Parent element in the hierarchy, or nullptr if root
        Element* parent = nullptr;
        /// @brief Owned child elements
        std::vector<OwnedElement> children;

        bool inherited = false;
        Element* layoutOwner = nullptr;

        //----------[ LAYOUT ]----------//

        /// @brief Absolute bounds computer from Yoga
        Rect<float> bounds = Rect<float>(0.0f, 0.0f, 0.0f, 0.0f);
        /// @brief Yoga node for this element
        YGNodeRef node = nullptr;
        /// @brief Shared Yoga config
        YGConfigRef config = nullptr;
        /// @brief True when the layout needs to be recalculated at the root
        bool layoutDirty = true;

        void refreshInheritanceCacheRecursive() {
            if (!inherited) layoutOwner = this;
            else {
                Element* a = parent;
                while(a && a->inherited) a = a->parent;
                layoutOwner = a ? a : this;
            }

            for (auto& c : children) c->refreshInheritanceCacheRecursive();
        }

        void clearLayoutDirtyRecursive();

        YGConfigRef createConfig();

        void makeNode();
        void removeNode();

    protected:
        void markLayoutDirty();
        bool needsLayout() const { return layoutDirty; }
        bool ownsLayout() const { return !inherited; }

        //----------[ HOOKS ]----------//

        virtual bool init(AssetManager& assetManager) { return true; }
        virtual void draw(Graphics& g) const {}
        virtual void update(int deltaTime) {}

    public:
        enum Type { Base, Circular };

        explicit Element(Element* parent);
        virtual ~Element();

        virtual Type getType() const { return Type::Base; }
        
        //----------[ TREE ]----------//

        const Element* getLayoutOwner() const { return layoutOwner ? layoutOwner : this; }
        Element* getLayoutOwner() { return layoutOwner ? layoutOwner : this; }

        Element* getParent() const { return parent; }
        YGNodeRef getNode() const { return node; }
        YGConfigRef getConfig() const { return config; }
        const Rect<float>& getBounds() const { return getLayoutOwner()->bounds; }

        void addChild(const rapidjson::Value::ConstObject json);
        bool removeChild(Element* child);
        void clearChildren();

        bool isRoot() const { return parent == nullptr; }

        //----------[ TRAVERSAL ]----------//

        bool initRecursive(AssetManager& assetManager);
        void drawRecursive(Graphics& g) const;
        void updateRecursive(int deltaTime);

        //----------[ LAYOUT ]----------//

        void layoutRecursive(float width, float height, YGDirection direction = YGDirectionLTR);

        //----------[ LAYOUT ]----------//
        
        static OwnedElement fromJson(Element *parent, const rapidjson::Value::ConstObject json);
       
        void loadFromJson(const rapidjson::Value::ConstObject& json) {
            loadLayout(json);
            loadProps(json);
            loadChildren(json);
        }

        void applyInheritance() {
            if (inherited) removeNode();
            else makeNode();

            refreshInheritanceCacheRecursive();
            markLayoutDirty();
        }

        void setInherited(bool newInherited) {
            if (!parent) newInherited = false; // To be inherited element MUST have parent
            if (inherited == newInherited) return;
            inherited = newInherited;
            applyInheritance();
        }

        static bool isInheritString(const rapidjson::Value::ConstObject& json) {
            auto it = json.FindMember("style");
            if (it == json.MemberEnd())
                return false;

            const rapidjson::Value& style = it->value;

            return style.IsString() && std::strcmp(style.GetString(), "inherit") == 0;
        }

        void loadLayout(const rapidjson::Value::ConstObject& json);

        void loadProps(const rapidjson::Value::ConstObject& json);

        void loadChildren(const rapidjson::Value::ConstObject& json);
};