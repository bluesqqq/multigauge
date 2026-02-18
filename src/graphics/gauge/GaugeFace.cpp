#include "GaugeFace.h"

GaugeFace::GaugeFace() : Element(nullptr) {}

GaugeFace::GaugeFace(const rapidjson::Document& json) : Element(nullptr, json.GetObject()) {
    if (!json.HasMember("props") || !json["props"].IsObject()) return;
    const rapidjson::Value::ConstObject props = json["props"].GetObject();

    if (props.HasMember("backgroundColor"))
        backgroundColor = Color::fromJson(props["backgroundColor"]);

    setCString(props, "title", title);
    setCString(props, "description", description);
}

void GaugeFace::draw(Graphics &g) const {
    if (backgroundColor) g.fillAll(*backgroundColor);
    else g.fillAll(rgb(0, 0, 0));
}
