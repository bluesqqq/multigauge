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

    return loadProperties(value);
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

void GaugeFace::update(int deltaTime) {
    for (auto& c : children) c->updateRecursive(deltaTime);
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

        return out;
    }

    return nullptr;
}

void GaugeFace::layout(Graphics& g) {
    if (!node) return;

    style.apply(node);

    const auto screenBounds = g.getScreenBounds();

    const float width  = screenBounds.width;
    const float height = screenBounds.height;

    YGNodeStyleSetWidth(node, width);
    YGNodeStyleSetHeight(node, height);

    YGNodeCalculateLayout(node, width, height, YGDirectionLTR);

    for (auto& c : children) c->layoutRecursive(0.0f, 0.0f);
}

} // namespace mg::gauge
