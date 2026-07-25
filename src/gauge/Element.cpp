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

namespace mg::gauge {

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

bool Element::setChildren(::mg::PropertyObject* obj, json::Reader value) {
    auto* self = static_cast<Element*>(obj);

    std::vector<OwnedElement> decoded;
    if (!decodeAny(value, decoded)) return false;

    while (!self->children.empty()) {
        self->removeChild(self->children.back().get());
    }

    for (auto& child : decoded) {
        if (!self->insertChild(std::move(child), self->children.size())) return false;
    }

    return true;
}

bool Element::getChildren(const ::mg::PropertyObject* obj, json::Writer& writer) {
    const auto* self = static_cast<const Element*>(obj);
    return encodeAny(writer, self->children);
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

void Element::updateRecursive(std::chrono::microseconds delta) {
    update(delta);
    for (auto& c : children) c->updateRecursive(delta);
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

} // namespace mg::gauge

namespace mg {

DECODE_IMPL(gauge::OwnedElement) {
    if (v.isNull()) {
        out = nullptr;
        return true;
    }

    if (!v.isObject()) return false;

    std::string_view typeView;
    (void)v.member("type").read(typeView);
    gauge::OwnedElement decoded = gauge::Element::registry().create(typeView, nullptr);
    if (!decoded || !decoded->loadProperties(v)) return false;
    out = std::move(decoded);
    return true;
}

ENCODE_IMPL(gauge::OwnedElement) {
    if (!v) {
        return out.null();
    }

    return v->saveProperties(out);
}

} // namespace mg
