#include <multigauge/App.h>
#include <multigauge/gauge/GaugeFace.h>

namespace mg::gauge {

GaugeFace::GaugeFace() : node(YGNodeNewWithConfig(mg::getYogaConfig())) {}

GaugeFace::~GaugeFace() {
    children.clear();

    if (node) {
        YGNodeFree(node);
        node = nullptr;
    }
}

bool GaugeFace::load(json::Reader value) {
    children.clear();
    backgroundColor.reset();
    style = RootLayout{};

    if (!value.isObject()) return false;

    const bool loaded = loadProperties(value);
    if (loaded) markLayoutDirty();
    return loaded;
}

bool GaugeFace::save(json::Writer& writer) const {
    return saveProperties(writer);
}

bool GaugeFace::setChildren(::mg::PropertyObject* obj, json::Reader value) {
    auto* self = static_cast<GaugeFace*>(obj);

    std::vector<OwnedElement> decoded;
    if (!decodeAny(value, decoded)) return false;

    for (const auto& child : decoded) {
        if (!child) return false;
    }

    while (!self->children.empty()) {
        self->removeChild(self->children.back().get());
    }

    for (auto& child : decoded) {
        if (!self->insertChild(std::move(child), self->children.size())) return false;
    }

    return true;
}

bool GaugeFace::getChildren(const ::mg::PropertyObject* obj, json::Writer& writer) {
    const auto* self = static_cast<const GaugeFace*>(obj);
    return encodeAny(writer, self->children);
}

bool GaugeFace::init(AssetManager& assetManager, GraphicsContext& context) {
    bool ok = true;

    for (auto& c : children) if (!c->initRecursive(assetManager, context)) ok = false;

    return ok;
}

void GaugeFace::update(std::chrono::microseconds delta) {
    for (auto& c : children) c->updateRecursive(delta);
}

void GaugeFace::draw(Graphics& g) const {
    g.fillAll(backgroundColor.get());

    for (const auto& c : children) c->drawRecursive(g);
}

bool GaugeFace::insertChild(OwnedElement child, std::size_t index) {
    if (!child) return false;

    Element* raw = child.get();
    const size_t i = std::min(index, children.size());

    raw->face = this;

    if (node && raw->getNode()) {
        if (YGNodeRef owner = YGNodeGetOwner(raw->getNode()); owner != node) {
            if (owner) YGNodeRemoveChild(owner, raw->getNode());
            YGNodeInsertChild(node, raw->getNode(), (uint32_t)i);
        }
    }

    children.insert(children.begin() + i, std::move(child));
    markLayoutDirty();
    return true;
}

OwnedElement GaugeFace::removeChild(Element *child) {
    if (!child) return nullptr;

    for (size_t i = 0; i < children.size(); ++i) {
        if (children[i].get() != child) continue;

        if (node && child->getNode() && YGNodeGetOwner(child->getNode()) == node) {
            YGNodeRemoveChild(node, child->getNode());
        }

        OwnedElement out = std::move(children[i]);
        children.erase(children.begin() + i);

        out->face = nullptr;
        markLayoutDirty();

        return out;
    }

    return nullptr;
}

void GaugeFace::layout(Graphics& g) {
    if (!node) return;

    const auto screenBounds = g.getScreenBounds();

    const float width  = screenBounds.width;
    const float height = screenBounds.height;

    const bool sizeChanged = width != layoutWidth || height != layoutHeight;
    if (!layoutDirty && !sizeChanged) return;

    if (rootLayoutDirty || sizeChanged) {
        style.apply(node);
        YGNodeStyleSetWidth(node, width);
        YGNodeStyleSetHeight(node, height);
    }

    for (auto& child : children) child->syncLayoutRecursive();
    YGNodeCalculateLayout(node, width, height, YGDirectionLTR);

    const auto updateBounds = [&](auto&& self, Element& element, float parentAbsX, float parentAbsY, bool parentChanged) -> void {
        const bool changed = parentChanged || YGNodeGetHasNewLayout(element.node);
        float absX = element.bounds.x;
        float absY = element.bounds.y;
        if (changed) {
            absX = parentAbsX + YGNodeLayoutGetLeft(element.node);
            absY = parentAbsY + YGNodeLayoutGetTop(element.node);
            element.bounds = Rect<float>(absX, absY, YGNodeLayoutGetWidth(element.node), YGNodeLayoutGetHeight(element.node));
            YGNodeSetHasNewLayout(element.node, false);
        }

        for (auto& child : element.children) self(self, *child, absX, absY, changed);
        element.subtreeLayoutDirty = false;
    };

    const bool rootChanged = sizeChanged || rootLayoutDirty;
    for (auto& child : children) updateBounds(updateBounds, *child, 0.0f, 0.0f, rootChanged);
    YGNodeSetHasNewLayout(node, false);

    layoutWidth = width;
    layoutHeight = height;
    rootLayoutDirty = false;
    layoutDirty = false;
}

void GaugeFace::markLayoutDirty() {
    rootLayoutDirty = true;
    layoutDirty = true;
}

void GaugeFace::markLayoutSubtreeDirty() {
    layoutDirty = true;
}

} // namespace mg::gauge
