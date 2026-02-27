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

        static bool styleIsInherit(const rapidjson::Value::ConstObject& json) {
            auto it = json.FindMember("style");
            if (it == json.MemberEnd()) return false;
            if (!it->value.IsString()) return false;
            return std::strcmp(it->value.GetString(), "inherit") == 0;
        }

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

        explicit Element(Element* parent, bool inherit = false);
        explicit Element(Element* parent, const rapidjson::Value::ConstObject json);
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

        bool isRoot() const { return parent == nullptr; }

        //----------[ TRAVERSAL ]----------//

        bool initRecursive(AssetManager& assetManager);
        void drawRecursive(Graphics& g) const;
        void updateRecursive(int deltaTime);

        //----------[ LAYOUT ]----------//

        void layoutRecursive(float width, float height, YGDirection direction = YGDirectionLTR);

        static OwnedElement fromJson(Element *parent, const rapidjson::Value::ConstObject json);
};