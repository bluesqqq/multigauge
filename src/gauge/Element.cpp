#include <multigauge/App.h>
#include <multigauge/gauge/Element.h>

#include <algorithm>

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

    OwnedElement createDefaultElement(Element* parent) {
        return std::make_unique<Element>(parent);
    }

    using ElementTypeDescriptor = MgPolymorphicTypeDescriptor<OwnedElement, Element*>;

    static const ElementTypeDescriptor ELEMENT_TYPES[] = {
        makePolymorphicTypeDescriptor<RectangleElement, OwnedElement, Element*>(&createElement<RectangleElement>),
        makePolymorphicTypeDescriptor<CircleElement, OwnedElement, Element*>(&createElement<CircleElement>),
        makePolymorphicTypeDescriptor<ImageElement, OwnedElement, Element*>(&createElement<ImageElement>),
        makePolymorphicTypeDescriptor<TextElement, OwnedElement, Element*>(&createElement<TextElement>),
        makePolymorphicTypeDescriptor<Horizon, OwnedElement, Element*>(&createElement<Horizon>),
        makePolymorphicTypeDescriptor<Graph, OwnedElement, Element*>(&createElement<Graph>),
        makePolymorphicTypeDescriptor<CircularElement, OwnedElement, Element*>(&createElement<CircularElement>),
        makePolymorphicTypeDescriptor<CircularNeedle, OwnedElement, Element*>(&createElement<CircularNeedle>),
        makePolymorphicTypeDescriptor<CircularScale, OwnedElement, Element*>(&createElement<CircularScale>),
    };
}

const Element::Registry& Element::registry() {
    static const Registry registry(ELEMENT_TYPES, sizeof(ELEMENT_TYPES) / sizeof(ELEMENT_TYPES[0]), &createDefaultElement);
    return registry;
}

Element::Element(Element* p) : parent(p), node(YGNodeNewWithConfig(mg::getYogaConfig())) {
    YGNodeSetContext(node, this);
}

Element::~Element() {
    children.clear();

    if (node) {
        YGNodeSetContext(node, nullptr);
        YGNodeFree(node);
        node = nullptr;
    }
}

namespace {
    std::size_t clampChildIndex(const Element* parent, std::size_t index) {
        return std::min(index, parent ? parent->childCount() : std::size_t{0});
    }
}

void Element::loadLayout(const rapidjson::Value::ConstObject &json) {
    auto it = json.FindMember("style");
    if (it != json.MemberEnd() && it->value.IsObject()) {
        style.loadProperties(it->value.GetObject());
    }
}

void Element::loadProps(const rapidjson::Value::ConstObject &json) {
    if (json.HasMember("props") && json["props"].IsObject())
        loadProperties(json["props"].GetObject());
}

void Element::loadChildren(const rapidjson::Value::ConstObject &json) {
    if (json.HasMember("children") && json["children"].IsArray()) {
        children.clear();
        for (const auto& childJson : json["children"].GetArray()) {
            OwnedElement child = Element::fromJson(childJson.GetObject());
            if (!child) continue;

            insertChild(std::move(child), children.size());
        }
    }
}

bool Element::insertChild(OwnedElement child, std::size_t index) {
    constexpr const char* TAG = "Element::insertChild";
    if (!child) {
        LOG_WARN(TAG, "Called with null child");
        return false;
    }

    Element* rawChild = child.get();
    const std::size_t childIndex = clampChildIndex(this, index);

    rawChild->parent = this;
    if (node && rawChild->node) {
        if (YGNodeRef owner = YGNodeGetOwner(rawChild->node); owner != node) {
            if (owner) {
                YGNodeRemoveChild(owner, rawChild->node);
            }
            YGNodeInsertChild(node, rawChild->node, static_cast<std::uint32_t>(childIndex));
        }
    }

    children.insert(children.begin() + static_cast<std::ptrdiff_t>(childIndex), std::move(child));
    return true;
}

OwnedElement Element::removeChild(Element* child) {
    constexpr const char* TAG = "Element::removeChild";
    if (!child) {
        LOG_WARN(TAG, "Called with null child");
        return nullptr;
    }

    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i].get() != child) continue;

        if (node && child->node && YGNodeGetOwner(child->node) == node)
            YGNodeRemoveChild(node, child->node);

        OwnedElement detached = std::move(children[i]);
        children.erase(children.begin() + static_cast<std::ptrdiff_t>(i));

        detached->parent = nullptr;
        detached->face = nullptr;
        
        return detached;
    }

    LOG_WARN(TAG, "Child=%p not found under parent=%p", (void*)child, (void*)this);
    return nullptr;
}

bool Element::initRecursive(AssetManager &assetManager, GraphicsContext& context) {
    bool success = init(assetManager, context);
    for (auto const& c : children) if (!c->initRecursive(assetManager, context)) success = false;
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

void Element::layoutRecursive(float parentAbsX, float parentAbsY) {
    style.apply(node);
    
    const float left = YGNodeLayoutGetLeft(node);
    const float top  = YGNodeLayoutGetTop(node);
    const float w    = YGNodeLayoutGetWidth(node);
    const float h    = YGNodeLayoutGetHeight(node);

    const float absX = parentAbsX + left;
    const float absY = parentAbsY + top;

    bounds = Rect<float>(absX, absY, w, h);

    for (auto& c : children)c->layoutRecursive(absX, absY);
}

void Element::saveToJson(rapidjson::Value& out, rapidjson::Document::AllocatorType& a) const {
    out.SetObject();

    if (const char* type = typeId()) {
        out.AddMember(rapidjson::Value(TYPE_KEY, a), rapidjson::Value(type, a), a);
    }

    rapidjson::Value styleValue;
    if (getProperty("style", styleValue, a)) {
        out.AddMember(rapidjson::Value("style", a), std::move(styleValue), a);
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

OwnedElement Element::fromJson(const rapidjson::Value::ConstObject json) {
    constexpr const char* TAG = "Element::fromJson";

    const char* type = nullptr;
    if (auto it = json.FindMember("type"); it != json.MemberEnd() && it->value.IsString())
        type = it->value.GetString();

    OwnedElement out;

    if (!type) {
        LOG_INFO(TAG, "No valid 'type'; constructing base Element.");
        out = std::make_unique<Element>(nullptr);
    } else {
        const auto* descriptor = registry().find(type);

        out = registry().create(type, nullptr);

        if (!descriptor) LOG_WARN(TAG, "Unknown type='%s'. Falling back to base Element.", type);
    }

    out->loadFromJson(json);

    return out;
}
