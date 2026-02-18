#include "Element.h"

#include "layout.h"

Element::Entry Element::registry[MAX_ELEMENT_REGISTRY_SIZE] = {};
size_t Element::registryCount = 0;

void Element::clearLayoutDirtyRecursive() {
    layoutDirty = false;
    for (auto& child : children) child->clearLayoutDirtyRecursive();
}

YGConfigRef Element::createConfig() {
    YGConfigRef config = YGConfigNew();
    YGConfigSetUseWebDefaults(config, false);
    return config;
}

void Element::markLayoutDirty() {
    Element* n = this;
    while (n->parent) n = n->parent;
    n->layoutDirty = true;
}

Element::Element(Element* p, bool inherit) : parent(p), inherited(inherit) {
    config = parent ? parent->getConfig() : createConfig();

    if (!parent) inherited = false; // root must own layout

    if (!inherited) {
        node = YGNodeNewWithConfig(config);
        YGNodeSetContext(node, this);
    }

    refreshInheritanceCacheRecursive();
}

Element::Element(Element* p, const rapidjson::Value::ConstObject json) : Element(p, styleIsInherit(json)) {
    if (!inherited) loadLayout(getNode(), json);

    if (!json.HasMember("children") || !json["children"].IsArray()) return;
    for (const auto& child : json["children"].GetArray())
        addChild(child.GetObject());
}

Element::~Element() {
    children.clear();
    
    if (node) {
        YGNodeFree(node);
        node = nullptr;
    }
}

#include "elements/primitives/TextElement.h"
#include "elements/primitives/RectangleElement.h"
#include "elements/primitives/CircleElement.h"
#include "elements/Horizon.h"
#include "elements/primitives/ImageElement.h"
#include "elements/Graph.h"
#include "elements/circular/CircularNeedle.h"
#include "elements/circular/CircularScale.h"

bool Element::registerType(const char *type, FactoryFn fn) {
    constexpr const char* TAG = "Element::registerType";
    if (!type || !fn) {
        LOG_ERROR(TAG, "Invalid registration (type=%p fn=%p)", (void*)type, (void*)fn);
        return false;
    }

    for (size_t i = 0; i < registryCount; ++i)
        if (registry[i].type && std::strcmp(registry[i].type, type) == 0) {
            LOG_WARN(TAG, "Duplicate type '%s' registered; overwriting factory", type);
            registry[i].fn = fn; 
            return true;
        }
    
    if (registryCount >= MAX_ELEMENT_REGISTRY_SIZE) {
        LOG_ERROR(TAG, "Registry full (%u/%u). Cannot register '%s'", (unsigned)registryCount, (unsigned)MAX_ELEMENT_REGISTRY_SIZE, type);
        return false;
    }

    registry[registryCount++] = {type, fn};
    LOG_DEBUG(TAG, "Registered '%s' (count=%u)", type, (unsigned)registryCount);
    return true;
}

std::unique_ptr<Element> Element::fromJson(Element *parent, const rapidjson::Value::ConstObject json) {
    constexpr const char* TAG = "Element::fromJson";

    if (!json.HasMember("type")) {
        LOG_INFO(TAG, "No 'type' field; constructing base Element.");
        return std::make_unique<Element>(parent, json);
    } 
    
    if (!json["type"].IsString()) {
        LOG_WARN(TAG, "'type' field exists but is not a string; falling back to base Element.");
        return std::make_unique<Element>(parent, json);
    } 

    const char* type = json["type"].GetString();

    if (registryCount == 0) LOG_ERROR(TAG, "Registry is empty. No element types registered. type='%s'", type);

    if (type) {
        for (size_t i = 0; i < registryCount; ++i) {
            if (!registry[i].type || !registry[i].fn) {
                LOG_WARN(TAG, "Bad registry entry %u (type=%p fn=%p)", (unsigned)i, (void*)registry[i].type, (void*)registry[i].fn);
                continue;
            }

            if (std::strcmp(registry[i].type, type) == 0) {
                LOG_INFO(TAG, "Matched type='%s'; constructing", type);
                return registry[i].fn(parent, json);
            }
        }
    }

    LOG_WARN(TAG, "Unknown type='%s' (registryCount=%u). Falling back to base Element.", type, (unsigned)registryCount);

    return std::make_unique<Element>(parent, json);
}

void Element::addChild(const rapidjson::Value::ConstObject json) {
    constexpr const char* TAG = "Element::addChild";

    std::unique_ptr<Element> child = fromJson(this, json);
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
    if (!layoutDirty) return;

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