#include <multigauge/gauge/GaugeFace.h>

void GaugeFace::load(const rapidjson::Document& doc) {
    if (!doc.IsObject()) return;
    const auto json = doc.GetObject();

    root.reset();

    if (json.HasMember("root") && json["root"].IsObject())
        root = Element::fromJson(nullptr, json["root"].GetObject());

    if (json.HasMember("props") && json["props"].IsObject())
        loadProperties(json["props"].GetObject());
}

rapidjson::Document GaugeFace::save() const {
    rapidjson::Document doc;

    doc.SetObject();
    auto& a = doc.GetAllocator();

    rapidjson::Value props(rapidjson::kObjectType);
    saveProperties(props, a);
    doc.AddMember("props", props, a);

    const Element* rootElement = getRoot();
    if (rootElement) {
        rapidjson::Value root(rapidjson::kObjectType);

        //TODO: save to root

        doc.AddMember("root", root, a);
    }

    return doc;
}

void GaugeFace::layout(Graphics &g) {
    auto screen = g.getScreenBounds().toFloat();

    root->layoutRecursive(screen.width, screen.height);
}

void GaugeFace::draw(Graphics &g) const {
    g.fillAll(backgroundColor.get());
    if (!root) return;
    root->drawRecursive(g);
}

void GaugeFace::update(int deltaTime) {
    if (!root) return;
    root->updateRecursive(deltaTime);
}

bool GaugeFace::init(AssetManager &assetManager) {
    if (!root) return false;
    return root->initRecursive(assetManager);
}
