#include <multigauge/gauge/Element.h>

#include <multigauge/layout.h>

void Element::clearLayoutDirtyRecursive() {
    layoutDirty = false;
    for (auto& child : children) child->clearLayoutDirtyRecursive();
}

YGConfigRef Element::createConfig() {
    YGConfigRef config = YGConfigNew();
    YGConfigSetUseWebDefaults(config, false);
    return config;
}

void Element::makeNode() {
    if (!node) {
        if (!config) config = parent ? parent->getConfig() : createConfig();
        node = YGNodeNewWithConfig(config);
        YGNodeSetContext(node, this);
    }
}

void Element::removeNode() {
    if (!node) return;

    if (parent) {
        Element* yogaParent = parent->getLayoutOwner();
        if (yogaParent && yogaParent->node) {
            YGNodeRemoveChild(yogaParent->node, node);
        }
    }

    YGNodeSetContext(node, nullptr);
    YGNodeFree(node);
    node = nullptr;
}

void Element::markLayoutDirty() {
    Element* n = this;
    while (n->parent) n = n->parent;
    n->layoutDirty = true;
}

Element::Element(Element* p) : parent(p) { applyInheritance(); }

Element::~Element() {
    children.clear();
    removeNode();
    if (!parent && config) {
        YGConfigFree(config);
        config = nullptr;
    }
}

void Element::loadLayout(const rapidjson::Value::ConstObject &json) {
    setInherited(parent ? isInheritString(json) : false);

    if (!inherited) loadYogaLayout(node, json);
}

void Element::loadProps(const rapidjson::Value::ConstObject &json) {
    if (json.HasMember("props") && json["props"].IsObject())
        loadProperties(json["props"].GetObject());
}

void Element::loadChildren(const rapidjson::Value::ConstObject &json) {
    if (json.HasMember("children") && json["children"].IsArray()) {
        children.clear();
        for (const auto& child : json["children"].GetArray())
            addChild(child.GetObject());
    }
}

void Element::addChild(const rapidjson::Value::ConstObject json) {
    constexpr const char* TAG = "Element::addChild";

    OwnedElement child = fromJson(this, json);
    if (!child) {
        LOG_ERROR(TAG, "fromJson returned nullptr; child skipped");
        return;
    }

    Element* yogaParent = this->getLayoutOwner();
    
    if (child->ownsLayout()) {
        if (yogaParent && yogaParent->node) {
            const uint32_t index = (uint32_t)YGNodeGetChildCount(yogaParent->node);
            YGNodeInsertChild(yogaParent->node, child->node, index);
        }
    }
    
    children.push_back(std::move(child));
    refreshInheritanceCacheRecursive();
    markLayoutDirty();
}

bool Element::removeChild(Element *child) {
    constexpr const char* TAG = "Element::removeChild";
    if (!child) {
        LOG_WARN(TAG, "Called with null child");
        return false;
    }

    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i].get() == child) {
            LOG_DEBUG(TAG, "Removing child=%p from parent=%p", (void*)child, (void*)parent);
            if (child->ownsLayout()) {
                Element* yogaParent = this->getLayoutOwner();
                if (yogaParent && yogaParent->node && child->node) {
                    YGNodeRemoveChild(yogaParent->node, child->node);
                } else {
                    LOG_WARN(TAG, "Could not remove Yoga node (yogaParent=%p yogaNode=%p childNode=%p)", (void*)yogaParent, yogaParent ? (void*)yogaParent->node : nullptr, (void*)child->node);
                }
            }

            child->parent = nullptr;
            children.erase(children.begin() + i);

            refreshInheritanceCacheRecursive();
            markLayoutDirty();
            return true;
        }
    }

    LOG_WARN(TAG, "Child=%p not found under parent=%p", (void*)child, (void*)this);
    return false;
}

bool Element::initRecursive(AssetManager &assetManager) {
    bool success = init(assetManager);
    for (auto const& c : children) if (!c->initRecursive(assetManager)) success = false;
    return success;
}

void Element::drawRecursive(Graphics &g) const {
    draw(g);
    for (auto const& c : children) c->drawRecursive(g);
}

void Element::updateRecursive(int deltaTime) {
    update(deltaTime);
    for (auto& c : children) c->updateRecursive(deltaTime);
}

void Element::layoutRecursive(float width, float height, YGDirection direction) {
    Element* root = this;
    while (root->parent) root = root->parent;

    Element* layoutRoot = root->getLayoutOwner(); // should always be root
    if (!layoutRoot->node) return;

    YGNodeCalculateLayout(layoutRoot->node, width, height, direction);
   
    auto walk = [&](auto&& self, Element* e, float parentAbsX, float parentAbsY) -> void {
        if (e->ownsLayout()) {
            const float left   = YGNodeLayoutGetLeft(e->node);
            const float top    = YGNodeLayoutGetTop(e->node);
            const float w      = YGNodeLayoutGetWidth(e->node);
            const float h      = YGNodeLayoutGetHeight(e->node);

            const float absX = parentAbsX + left;
            const float absY = parentAbsY + top;

            e->bounds = Rect<float>(absX, absY, w, h);

            for (auto& c : e->children) self(self, c.get(), absX, absY);
        } else {
            for (auto& c : e->children) self(self, c.get(), parentAbsX, parentAbsY);
        }
    };

    walk(walk, root, 0, 0);

    root->clearLayoutDirtyRecursive();
}

#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/gauge/elements/primitives/TextElement.h>
#include <multigauge/gauge/elements/primitives/RectangleElement.h>
#include <multigauge/gauge/elements/primitives/CircleElement.h>
#include <multigauge/gauge/elements/primitives/ImageElement.h>
#include <multigauge/gauge/elements/Horizon.h>
#include <multigauge/gauge/elements/Graph.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>
#include <multigauge/gauge/elements/circular/CircularNeedle.h>
#include <multigauge/gauge/elements/circular/CircularScale.h>

OwnedElement Element::fromJson(Element *parent, const rapidjson::Value::ConstObject json) {
    constexpr const char* TAG = "Element::fromJson";

    const char* type = nullptr;
    if (auto it = json.FindMember("type"); it != json.MemberEnd() && it->value.IsString())
        type = it->value.GetString();

    OwnedElement out;

    if (!type) {
        LOG_INFO(TAG, "No valid 'type'; constructing base Element.");
        out = std::make_unique<Element>(parent);
    } else {
        if (std::strcmp(type, "rectangle")             == 0) out = std::make_unique<RectangleElement>(parent);
        else if (std::strcmp(type, "circle")           == 0) out = std::make_unique<CircleElement>(parent);
        else if (std::strcmp(type, "image")            == 0) out = std::make_unique<ImageElement>(parent);
        else if (std::strcmp(type, "text")             == 0) out = std::make_unique<TextElement>(parent);
        else if (std::strcmp(type, "horizon")          == 0) out = std::make_unique<Horizon>(parent);
        else if (std::strcmp(type, "graph")            == 0) out = std::make_unique<Graph>(parent);
        else if (std::strcmp(type, "circular-element") == 0) out = std::make_unique<CircularElement>(parent);
        else if (std::strcmp(type, "circular-needle")  == 0) out = std::make_unique<CircularNeedle>(parent);
        else if (std::strcmp(type, "circular-scale")   == 0) out = std::make_unique<CircularScale>(parent);
        else {
            LOG_WARN(TAG, "Unknown type='%s'. Falling back to base Element.", type);
            out = std::make_unique<Element>(parent);
        }
    }

    out->loadFromJson(json);

    return out;
}