#include <multigauge/gauge/GaugeFace.h>

GaugeFace::GaugeFace() : Element(nullptr) {}

GaugeFace::GaugeFace(const rapidjson::Document& json) : Element(nullptr, json.GetObject()) {}

void GaugeFace::draw(Graphics &g) const {
    g.fillAll(backgroundColor.get());
}
