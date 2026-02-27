#include <multigauge/gauge/GaugeFace.h>

GaugeFace::GaugeFace() : Element(nullptr) {}

void GaugeFace::draw(Graphics &g) const {
    g.fillAll(backgroundColor.get());
}
