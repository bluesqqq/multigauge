#include <multigauge/App.h>
#include <multigauge/gauge/GaugeFace.h>

GaugeFace::GaugeFace() : node(YGNodeNewWithConfig(mg::getYogaConfig())) {}

GaugeFace::~GaugeFace() {
    children.clear();

    if (node) {
        YGNodeFree(node);
        node = nullptr;
    }
}

void GaugeFace::load(const rapidjson::Value& json) {
    children.clear();

    if (!json.IsObject()) return;

    if (json.HasMember("layout") && json["layout"].IsObject()) {
        style.loadProperties(json["layout"].GetObject());
    }

    if (json.HasMember("children") && json["children"].IsArray()) {
        for (const auto& c : json["children"].GetArray()) {
            auto child = Element::fromJson(c.GetObject());
            if (!child) continue;

            insertChild(std::move(child), children.size());
        }
    }
}

rapidjson::Document GaugeFace::save() const {
    rapidjson::Document doc;
    doc.SetObject();

    auto& a = doc.GetAllocator();

    rapidjson::Value layoutValue;
    style.saveProperties(layoutValue, a);
    doc.AddMember("layout", std::move(layoutValue), a);

    rapidjson::Value arr(rapidjson::kArrayType);

    for (const auto& c : children) {
        rapidjson::Value v;
        c->saveToJson(v, a);
        arr.PushBack(std::move(v), a);
    }

    doc.AddMember("children", std::move(arr), a);

    return doc;
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

    YGNodeCalculateLayout(node, width, height, YGDirectionLTR);

    for (auto& c : children) c->layoutRecursive(0.0f, 0.0f);
}