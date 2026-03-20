#include <multigauge/gauge/Element.h>

#include <algorithm>

#include <multigauge/layout.h>

#include <multigauge/gauge/elements/primitives/TextElement.h>
#include <multigauge/gauge/elements/primitives/RectangleElement.h>
#include <multigauge/gauge/elements/primitives/CircleElement.h>
#include <multigauge/gauge/elements/primitives/ImageElement.h>
#include <multigauge/gauge/elements/Horizon.h>
#include <multigauge/gauge/elements/Graph.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>
#include <multigauge/gauge/elements/circular/CircularNeedle.h>
#include <multigauge/gauge/elements/circular/CircularScale.h>

namespace {
    template <typename T>
    OwnedElement createElement(Element* parent) {
        return std::make_unique<T>(parent);
    }
}

const std::vector<ElementDescriptor>& Element::registry() {
    static const std::vector<ElementDescriptor> descriptors = {
        {"Rectangle", "rectangle", &createElement<RectangleElement>},
        {"Circle", "circle", &createElement<CircleElement>},
        {"Image", "image", &createElement<ImageElement>},
        {"Text", "text", &createElement<TextElement>},
        {"Horizon", "horizon", &createElement<Horizon>},
        {"Graph", "graph", &createElement<Graph>},
        {"Circular Element", "circular-element", &createElement<CircularElement>},
        {"Circular Needle", "circular-needle", &createElement<CircularNeedle>},
        {"Circular Scale", "circular-scale", &createElement<CircularScale>},
    };

    return descriptors;
}

const ElementDescriptor * Element::findDescriptor(const char * type) {
    if (!type) return nullptr;

    for (const auto& descriptor : registry()) {
        if (std::strcmp(descriptor.type, type) == 0) {
            return &descriptor;
        }
    }

    return nullptr;
}

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

namespace {
    std::size_t clampChildIndex(const Element* parent, std::size_t index) {
        return std::min(index, parent ? parent->childCount() : std::size_t{0});
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

Element* Element::addChild(const rapidjson::Value::ConstObject json) {
    return insertChild(json, childCount());
}

Element* Element::insertChild(const rapidjson::Value::ConstObject json, std::size_t index) {
    constexpr const char* TAG = "Element::addChild";

    OwnedElement child = fromJson(this, json);
    if (!child) {
        LOG_ERROR(TAG, "fromJson returned nullptr; child skipped");
        return nullptr;
    }

    Element* rawChild = child.get();
    if (!insertChild(std::move(child), index)) {
        LOG_ERROR(TAG, "insertChild returned false; child skipped");
        return nullptr;
    }

    return rawChild;
}

bool Element::insertChild(OwnedElement child, std::size_t index) {
    constexpr const char* TAG = "Element::insertChild";
    if (!child) {
        LOG_WARN(TAG, "Called with null child");
        return false;
    }

    Element* rawChild = child.get();
    const std::size_t childIndex = clampChildIndex(this, index);
    Element* yogaParent = this->getLayoutOwner();

    rawChild->parent = this;

    if (rawChild->ownsLayout()) {
        if (yogaParent && yogaParent->node && rawChild->node) {
            uint32_t insertIndex = 0;
            for (std::size_t i = 0; i < childIndex; ++i) {
                Element* sibling = children[i].get();
                if (sibling && sibling->ownsLayout()) {
                    ++insertIndex;
                }
            }
            YGNodeInsertChild(yogaParent->node, rawChild->node, insertIndex);
        }
    }

    children.insert(children.begin() + static_cast<std::ptrdiff_t>(childIndex), std::move(child));
    refreshInheritanceCacheRecursive();
    markLayoutDirty();
    return true;
}

OwnedElement Element::detachChild(Element* child) {
    constexpr const char* TAG = "Element::detachChild";
    if (!child) {
        LOG_WARN(TAG, "Called with null child");
        return nullptr;
    }

    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i].get() != child) continue;

        LOG_DEBUG(TAG, "Detaching child=%p from parent=%p", (void*)child, (void*)this);
        if (child->ownsLayout()) {
            Element* yogaParent = this->getLayoutOwner();
            if (yogaParent && yogaParent->node && child->node) {
                YGNodeRemoveChild(yogaParent->node, child->node);
            } else {
                LOG_WARN(TAG, "Could not remove Yoga node (yogaParent=%p yogaNode=%p childNode=%p)", (void*)yogaParent, yogaParent ? (void*)yogaParent->node : nullptr, (void*)child->node);
            }
        }

        child->parent = nullptr;
        OwnedElement detached = std::move(children[i]);
        children.erase(children.begin() + static_cast<std::ptrdiff_t>(i));

        refreshInheritanceCacheRecursive();
        markLayoutDirty();
        return detached;
    }

    LOG_WARN(TAG, "Child=%p not found under parent=%p", (void*)child, (void*)this);
    return nullptr;
}

bool Element::removeChild(Element *child) {
    return static_cast<bool>(detachChild(child));
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

void Element::saveToJson(rapidjson::Value& out, rapidjson::Document::AllocatorType& a) const {
    out.SetObject();

    if (const char* type = typeId()) {
        out.AddMember(rapidjson::Value(TYPE_KEY, a), rapidjson::Value(type, a), a);
    }

    if (inherited) {
        out.AddMember(rapidjson::Value("style", a), rapidjson::Value("inherit", a), a);
    } else {
        rapidjson::Value styleValue;
        if (saveProperty("style", styleValue, a)) {
            out.AddMember(rapidjson::Value("style", a), std::move(styleValue), a);
        }
    }

    rapidjson::Value props(rapidjson::kObjectType);
    propertyList().forEach(this, [&](const Property& property) {
        if (!property.key || !property.get) return;
        if (findProperty(property.key) != &property) return;
        if (std::strcmp(property.key, "style") == 0) return;

        rapidjson::Value value;
        if (!property.get(this, value, a)) return;

        props.AddMember(rapidjson::Value(property.key, a), std::move(value), a);
    });
    out.AddMember(rapidjson::Value("props", a), std::move(props), a);

    rapidjson::Value childArray(rapidjson::kArrayType);
    childArray.Reserve(static_cast<rapidjson::SizeType>(children.size()), a);
    for (const auto& child : children) {
        if (!child) continue;
        rapidjson::Value childValue;
        child->saveToJson(childValue, a);
        childArray.PushBack(std::move(childValue), a);
    }
    out.AddMember(rapidjson::Value("children", a), std::move(childArray), a);
}
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
        const ElementDescriptor* descriptor = findDescriptor(type);
        if (descriptor && descriptor->create) {
            out = descriptor->create(parent);
        } else {
            LOG_WARN(TAG, "Unknown type='%s'. Falling back to base Element.", type);
            out = std::make_unique<Element>(parent);
        }
    }

    out->loadFromJson(json);

    return out;
}
