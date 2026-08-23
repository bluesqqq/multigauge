#include <multigauge/gauge/GaugeFace.h>
#include <multigauge/graphics/Graphics.h>

#define CLAY_IMPLEMENTATION
#include <clay.h>

#include <algorithm>
#include <atomic>
#include <limits>

#include <multigauge/gauge/elements/FrameElement.h>
#include <multigauge/gauge/elements/circular/CircularElement.h>

#ifndef MG_GAUGE_MAX_LAYOUT_ELEMENTS
#define MG_GAUGE_MAX_LAYOUT_ELEMENTS 64
#endif

namespace mg::gauge {

namespace {

Clay_SizingAxis toClaySize(layout::Size size) {
    switch (size.mode) {
    case layout::SizeMode::Fit:
        return CLAY_SIZING_FIT(size.value, size.limit);
    case layout::SizeMode::Grow:
        return CLAY_SIZING_GROW(size.value, size.limit);
    case layout::SizeMode::Fixed:
        return CLAY_SIZING_FIXED(size.value);
    case layout::SizeMode::Percent:
        return CLAY_SIZING_PERCENT(size.value);
    }

    return CLAY_SIZING_FIT(0, 0);
}

Clay_LayoutAlignmentX toClayAlignment(layout::AlignmentX alignment) {
    switch (alignment) {
    case layout::AlignmentX::Left:
        return CLAY_ALIGN_X_LEFT;
    case layout::AlignmentX::Center:
        return CLAY_ALIGN_X_CENTER;
    case layout::AlignmentX::Right:
        return CLAY_ALIGN_X_RIGHT;
    }

    return CLAY_ALIGN_X_LEFT;
}

Clay_LayoutAlignmentY toClayAlignment(layout::AlignmentY alignment) {
    switch (alignment) {
    case layout::AlignmentY::Top:
        return CLAY_ALIGN_Y_TOP;
    case layout::AlignmentY::Center:
        return CLAY_ALIGN_Y_CENTER;
    case layout::AlignmentY::Bottom:
        return CLAY_ALIGN_Y_BOTTOM;
    }

    return CLAY_ALIGN_Y_TOP;
}

Clay_FloatingAttachPointType toClayAnchor(layout::FloatingAnchor anchor) {
    switch (anchor) {
    case layout::FloatingAnchor::LeftTop:
        return CLAY_ATTACH_POINT_LEFT_TOP;
    case layout::FloatingAnchor::LeftCenter:
        return CLAY_ATTACH_POINT_LEFT_CENTER;
    case layout::FloatingAnchor::LeftBottom:
        return CLAY_ATTACH_POINT_LEFT_BOTTOM;
    case layout::FloatingAnchor::CenterTop:
        return CLAY_ATTACH_POINT_CENTER_TOP;
    case layout::FloatingAnchor::Center:
        return CLAY_ATTACH_POINT_CENTER_CENTER;
    case layout::FloatingAnchor::CenterBottom:
        return CLAY_ATTACH_POINT_CENTER_BOTTOM;
    case layout::FloatingAnchor::RightTop:
        return CLAY_ATTACH_POINT_RIGHT_TOP;
    case layout::FloatingAnchor::RightCenter:
        return CLAY_ATTACH_POINT_RIGHT_CENTER;
    case layout::FloatingAnchor::RightBottom:
        return CLAY_ATTACH_POINT_RIGHT_BOTTOM;
    }

    return CLAY_ATTACH_POINT_LEFT_TOP;
}

Clay_FloatingAttachToElement toClayAttachTo(layout::FloatingAttachTo attachTo) {
    switch (attachTo) {
    case layout::FloatingAttachTo::None:
        return CLAY_ATTACH_TO_NONE;
    case layout::FloatingAttachTo::Parent:
        return CLAY_ATTACH_TO_PARENT;
    case layout::FloatingAttachTo::Root:
        return CLAY_ATTACH_TO_ROOT;
    }

    return CLAY_ATTACH_TO_NONE;
}

Clay_LayoutConfig toClayLayout(const layout::Layout& layoutState) {
    return {
        .sizing = {toClaySize(layoutState.width), toClaySize(layoutState.height)},
        .padding =
            {
                .left = static_cast<std::uint16_t>(std::max(0, layoutState.padding.left)),
                .right = static_cast<std::uint16_t>(std::max(0, layoutState.padding.right)),
                .top = static_cast<std::uint16_t>(std::max(0, layoutState.padding.top)),
                .bottom = static_cast<std::uint16_t>(std::max(0, layoutState.padding.bottom)),
            },
        .childGap = static_cast<std::uint16_t>(std::max(0, layoutState.childGap)),
        .childAlignment =
            {
                .x = toClayAlignment(layoutState.childAlignment.x),
                .y = toClayAlignment(layoutState.childAlignment.y),
            },
        .layoutDirection = layoutState.direction == layout::Direction::LeftToRight
                               ? CLAY_LEFT_TO_RIGHT
                               : CLAY_TOP_TO_BOTTOM,
    };
}

Clay_FloatingElementConfig toClayFloating(const layout::Floating& floating) {
    return {
        .offset = {floating.offset.x, floating.offset.y},
        .expand = {floating.expand.width, floating.expand.height},
        .zIndex = static_cast<std::int16_t>(std::clamp(
            floating.zIndex,
            static_cast<int>(std::numeric_limits<std::int16_t>::min()),
            static_cast<int>(std::numeric_limits<std::int16_t>::max()))),
        .attachPoints =
            {
                .element = toClayAnchor(floating.elementAnchor),
                .parent = toClayAnchor(floating.parentAnchor),
            },
        .attachTo = toClayAttachTo(floating.attachTo),
    };
}

Clay_ElementId clayId(NodeHandle node, const Element& element) {
    const char* rawTypeId = element.typeId();
    const std::string_view typeId = rawTypeId && *rawTypeId ? rawTypeId : "element";
    return Clay_GetElementIdWithIndex(
        {
            .isStaticallyAllocated = true,
            .length = static_cast<std::int32_t>(typeId.size()),
            .chars = typeId.data(),
        },
        node.slot());
}

std::uint32_t nextNodeGeneration() noexcept {
    static std::atomic<std::uint32_t> next{1};
    const std::uint32_t generation = next.fetch_add(1, std::memory_order_relaxed) & 0xffffu;
    return generation == 0 ? 1 : generation;
}

} // namespace

GaugeFace::GaugeFace() : nodes_(nextNodeGeneration()) {
    constexpr std::size_t maxElements = MG_GAUGE_MAX_LAYOUT_ELEMENTS;
    const std::size_t requestedElements = maxElements;
    const std::size_t cappedElements =
        std::min<std::size_t>(
            requestedElements,
            static_cast<std::size_t>(std::numeric_limits<std::int32_t>::max() - 1)) +
        1;

    // Clay keeps a process-global current context. Clear any previously
    // destroyed face before configuring defaults for this new instance.
    Clay_SetCurrentContext(nullptr);
    Clay_SetMaxElementCount(static_cast<std::int32_t>(cappedElements));
    // Clay's internal hash tables require a non-zero text cache capacity even
    // when this prototype does not declare text elements yet.
    Clay_SetMaxMeasureTextCacheWordCount(16);

    clayMemory_.resize(Clay_MinMemorySize());
    const Clay_Arena arena =
        Clay_CreateArenaWithCapacityAndMemory(clayMemory_.size(), clayMemory_.data());
    clayContext_ = Clay_Initialize(arena, {0.0f, 0.0f}, {});
}

GaugeFace::~GaugeFace() {
    if (Clay_GetCurrentContext() == static_cast<Clay_Context*>(clayContext_)) {
        Clay_SetCurrentContext(nullptr);
    }
}

NodeHandle GaugeFace::createElement(std::string_view typeId) {
    return addElement(Element::registry().create(typeId));
}

NodeHandle GaugeFace::addElement(std::unique_ptr<Element> element) {
    if (!element) return NodeHandle::invalid();

    const NodeHandle created = nodes_.emplace(std::move(element));
    appendRoot(created);
    return created;
}

bool GaugeFace::deleteElement(NodeHandle root) {
    Node* rootNode = node(root);
    if (!rootNode) return false;

    unlink(root);

    NodeHandle child = rootNode->firstChild;
    while (child.valid()) {
        Node* childNode = node(child);
        const NodeHandle next = childNode ? childNode->nextSibling : NodeHandle::invalid();
        if (childNode) deleteElement(child);
        child = next;
    }

    return nodes_.remove(root);
}

bool GaugeFace::replaceElement(NodeHandle handle, std::unique_ptr<Element> element) {
    Node* value = node(handle);
    if (!value || !element) return false;
    value->element = std::move(element);
    return true;
}

bool GaugeFace::moveElement(NodeHandle element, NodeHandle parent, std::size_t index) {
    return parent.valid()
               ? insertChild(parent, element, index)
               : insertRoot(element, index);
}

bool GaugeFace::insertChild(NodeHandle parent, NodeHandle child, std::size_t index) {
    Node* parentNode = node(parent);
    Node* childNode = node(child);
    if (!parentNode || !childNode || parent == child || wouldCreateCycle(parent, child))
        return false;

    unlink(child);
    parentNode = node(parent);
    childNode = node(child);

    NodeHandle* link = &parentNode->firstChild;
    while (index > 0 && link->valid()) {
        Node* sibling = node(*link);
        if (!sibling) return false;
        link = &sibling->nextSibling;
        --index;
    }

    childNode->parent = parent;
    childNode->nextSibling = *link;
    *link = child;
    return true;
}

bool GaugeFace::insertRoot(NodeHandle child, std::size_t index) {
    if (!node(child)) return false;

    unlink(child);
    Node* childNode = node(child);
    if (!childNode) return false;

    NodeHandle* link = &firstRoot_;
    while (index > 0 && link->valid()) {
        Node* sibling = node(*link);
        if (!sibling) return false;
        link = &sibling->nextSibling;
        --index;
    }

    childNode->parent.clear();
    childNode->nextSibling = *link;
    *link = child;
    return true;
}

Element* GaugeFace::get(NodeHandle nodeHandle) noexcept {
    Node* value = node(nodeHandle);
    return value ? value->element.get() : nullptr;
}

const Element* GaugeFace::get(NodeHandle nodeHandle) const noexcept {
    const Node* value = node(nodeHandle);
    return value ? value->element.get() : nullptr;
}

NodeHandle GaugeFace::parentOf(NodeHandle nodeHandle) const noexcept {
    const Node* value = node(nodeHandle);
    return value ? value->parent : NodeHandle::invalid();
}

bool GaugeFace::load(json::Reader value) {
    if (!value.isObject()) return false;

    while (firstRoot_.valid()) {
        if (!deleteElement(firstRoot_)) return false;
    }
    layout_ = {};
    backgroundColor_.reset();
    return loadProperties(value);
}

bool GaugeFace::save(json::Writer& writer) const {
    return saveProperties(writer);
}

void GaugeFace::update(std::chrono::microseconds delta) {
    if (clayContext_) {
        Clay_SetCurrentContext(static_cast<Clay_Context*>(clayContext_));
        Clay_UpdateScrollContainers(false,
                                    {0.0f, 0.0f},
                                    static_cast<float>(delta.count()) / 1'000'000.0f);
    }

    for (NodeHandle root = firstRoot_; root.valid();) {
        Node* rootNode = node(root);
        const NodeHandle next = rootNode ? rootNode->nextSibling : NodeHandle::invalid();
        if (rootNode) updateSubtree(root, delta);
        root = next;
    }
}

bool GaugeFace::init(std::string_view packageId, ::mg::AssetManager& assetManager, ::mg::graphics::GraphicsContext& context) {
    bool result = true;
    for (NodeHandle root = firstRoot_; root.valid();) {
        const Node* rootNode = node(root);
        const NodeHandle next = rootNode ? rootNode->nextSibling : NodeHandle::invalid();
        if (rootNode && !initSubtree(root, packageId, assetManager, context)) result = false;
        root = next;
    }
    return result;
}

GaugeFace::Node* GaugeFace::node(NodeHandle handle) noexcept {
    return nodes_.get(handle);
}

const GaugeFace::Node* GaugeFace::node(NodeHandle handle) const noexcept {
    return nodes_.get(handle);
}

bool GaugeFace::wouldCreateCycle(NodeHandle parent, NodeHandle child) const noexcept {
    for (NodeHandle current = parent; current.valid();) {
        if (current == child) return true;
        const Node* currentNode = node(current);
        if (!currentNode) return true;
        current = currentNode->parent;
    }
    return false;
}

void GaugeFace::updateSubtree(NodeHandle root, std::chrono::microseconds delta) {
    Node* rootNode = node(root);
    if (!rootNode) return;

    resolveInherited(*rootNode);
    rootNode->element->update(delta);

    for (NodeHandle child = rootNode->firstChild; child.valid();) {
        Node* childNode = node(child);
        const NodeHandle next = childNode ? childNode->nextSibling : NodeHandle::invalid();
        if (childNode) updateSubtree(child, delta);
        child = next;
    }
}

bool GaugeFace::initSubtree(NodeHandle root,
                            std::string_view packageId,
                            ::mg::AssetManager& assetManager,
                            ::mg::graphics::GraphicsContext& context) {
    Node* rootNode = node(root);
    if (!rootNode) return false;
    bool result = rootNode->element->init(packageId, assetManager, context);
    for (NodeHandle child = rootNode->firstChild; child.valid();) {
        Node* childNode = node(child);
        const NodeHandle next = childNode ? childNode->nextSibling : NodeHandle::invalid();
        if (childNode && !initSubtree(child, packageId, assetManager, context)) result = false;
        child = next;
    }
    return result;
}

void GaugeFace::resolveInherited(Node& nodeValue) {
    auto* circular = dynamic_cast<CircularElement*>(nodeValue.element.get());
    if (!circular) return;

    ::mg::ValueView inheritedValue;
    float inheritedStartAngle = 0.0f;
    float inheritedEndAngle = 360.0f;
    if (const Node* parentNode = node(nodeValue.parent)) {
        if (const auto* parent = dynamic_cast<const CircularElement*>(parentNode->element.get())) {
            inheritedValue = parent->resolvedValueView();
            inheritedStartAngle = parent->resolvedStartAngle();
            inheritedEndAngle = parent->resolvedEndAngle();
        }
    }
    circular->resolveInherited(inheritedValue, inheritedStartAngle, inheritedEndAngle);
}

void GaugeFace::declareClaySubtree(NodeHandle root) const {
    const Node* rootNode = node(root);
    if (!rootNode) return;

    CLAY({
        .id = clayId(root, *rootNode->element),
        .layout = toClayLayout(rootNode->element->layout()),
        .aspectRatio = {.aspectRatio = rootNode->element->layout().aspectRatio},
        .floating = toClayFloating(rootNode->element->layout().floating),
    }) {
        for (NodeHandle child = rootNode->firstChild; child.valid();) {
            const Node* childNode = node(child);
            const NodeHandle next = childNode ? childNode->nextSibling : NodeHandle::invalid();
            if (childNode) declareClaySubtree(child);
            child = next;
        }
    }
}

void GaugeFace::updateBoundsSubtree(NodeHandle root) {
    Node* rootNode = node(root);
    if (!rootNode) return;

    const layout::Layout& layoutState = rootNode->element->layout();
    const Node* parentNode = node(rootNode->parent);
    if (layoutState.floating.fillParent &&
        layoutState.floating.attachTo == layout::FloatingAttachTo::Parent && parentNode) {
        rootNode->bounds = parentNode->bounds;
    } else {
        const Clay_ElementData data = Clay_GetElementData(clayId(root, *rootNode->element));
        if (data.found) {
            rootNode->bounds = {
                data.boundingBox.x,
                data.boundingBox.y,
                data.boundingBox.width,
                data.boundingBox.height,
            };
        }
    }

    for (NodeHandle child = rootNode->firstChild; child.valid();) {
        Node* childNode = node(child);
        const NodeHandle next = childNode ? childNode->nextSibling : NodeHandle::invalid();
        if (childNode) updateBoundsSubtree(child);
        child = next;
    }
}

void GaugeFace::appendRoot(NodeHandle root) {
    Node* rootNode = node(root);
    if (!rootNode) return;

    rootNode->parent.clear();
    rootNode->nextSibling.clear();

    NodeHandle* link = &firstRoot_;
    while (link->valid()) {
        Node* sibling = node(*link);
        if (!sibling) return;
        link = &sibling->nextSibling;
    }
    *link = root;
}

void GaugeFace::unlink(NodeHandle child) {
    Node* childNode = node(child);
    if (!childNode) return;

    NodeHandle* link = nullptr;
    if (childNode->parent.valid()) {
        Node* parentNode = node(childNode->parent);
        if (parentNode) link = &parentNode->firstChild;
    } else {
        link = &firstRoot_;
    }

    if (link) {
        while (link->valid() && *link != child) {
            Node* sibling = node(*link);
            if (!sibling) break;
            link = &sibling->nextSibling;
        }

        if (*link == child) *link = childNode->nextSibling;
    }

    childNode->parent.clear();
    childNode->nextSibling.clear();
}

bool GaugeFace::setChildren(::mg::PropertyObject* object, json::Reader value) {
    auto* face = static_cast<GaugeFace*>(object);
    while (face->firstRoot_.valid()) {
        if (!face->deleteElement(face->firstRoot_)) return false;
    }
    return face->loadChildren(value, NodeHandle::invalid());
}

bool GaugeFace::getChildren(const ::mg::PropertyObject* object, json::Writer& writer) {
    return static_cast<const GaugeFace*>(object)->saveChildren(writer, NodeHandle::invalid());
}

bool GaugeFace::loadChildren(json::Reader value, NodeHandle parent) {
    if (!value.isArray()) return false;
    for (std::size_t i = 0; i < value.size(); ++i) {
        const json::Reader entry = value.element(i);
        if (!entry.isObject()) return false;

        Element::OwnedElement element;
        if (!decodeAny(entry, element) || !element) return false;

        const NodeHandle child = addElement(std::move(element));
        if (parent.valid() &&
            !insertChild(parent, child, std::numeric_limits<std::size_t>::max())) {
            deleteElement(child);
            return false;
        }
        const json::Reader children = entry.member("children");
        if (children.valid() && !loadChildren(children, child)) {
            deleteElement(child);
            return false;
        }
    }
    return true;
}

bool GaugeFace::saveChildren(json::Writer& writer, NodeHandle parent) const {
    return writer.writeArray([&](json::ArrayWriter& array) {
        NodeHandle current = parent.valid() ? node(parent)->firstChild : firstRoot_;
        while (current.valid()) {
            const Node* currentNode = node(current);
            if (!currentNode) return false;
            if (!array.writeObject([&](json::ObjectWriter& object) {
                    return currentNode->element->savePropertyMembers(object) &&
                           object.writeValue("children", [&](json::Writer& childWriter) {
                               return saveChildren(childWriter, current);
                           });
                }))
                return false;
            current = currentNode->nextSibling;
        }
        return true;
    });
}

void GaugeFace::draw(::mg::graphics::Graphics& graphics) {
    graphics.fillAll(backgroundColor_.get());

    for (NodeHandle root = firstRoot_; root.valid();) {
        Node* rootNode = node(root);
        const NodeHandle next = rootNode ? rootNode->nextSibling : NodeHandle::invalid();
        if (rootNode) drawSubtree(root, graphics);
        root = next;
    }
}

void GaugeFace::layout(::mg::graphics::Graphics& graphics) {
    const Rect<int> screen = graphics.getScreenBounds();
    if (!clayContext_) return;

    Clay_SetCurrentContext(static_cast<Clay_Context*>(clayContext_));
    Clay_SetLayoutDimensions({static_cast<float>(screen.width), static_cast<float>(screen.height)});
    Clay_BeginLayout();

    CLAY({
        .id = CLAY_ID("NewGaugeFace"),
        .layout = toClayLayout(layout_),
    }) {
        for (NodeHandle root = firstRoot_; root.valid();) {
            const Node* rootNode = node(root);
            const NodeHandle next = rootNode ? rootNode->nextSibling : NodeHandle::invalid();
            if (rootNode) declareClaySubtree(root);
            root = next;
        }
    }

    (void)Clay_EndLayout();

    for (NodeHandle root = firstRoot_; root.valid();) {
        const Node* rootNode = node(root);
        const NodeHandle next = rootNode ? rootNode->nextSibling : NodeHandle::invalid();
        if (rootNode) updateBoundsSubtree(root);
        root = next;
    }
}

void GaugeFace::drawSubtree(NodeHandle root, ::mg::graphics::Graphics& graphics) {
    Node* rootNode = node(root);
    if (!rootNode) return;

    resolveInherited(*rootNode);
    rootNode->element->draw(graphics, rootNode->bounds);

    for (NodeHandle child = rootNode->firstChild; child.valid();) {
        Node* childNode = node(child);
        const NodeHandle next = childNode ? childNode->nextSibling : NodeHandle::invalid();
        if (childNode) drawSubtree(child, graphics);
        child = next;
    }
}

} // namespace mg::gauge
