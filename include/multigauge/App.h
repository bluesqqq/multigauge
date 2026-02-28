#pragma once

class Element;

namespace mg {
    bool init(const char* gaugePath);
    void frame();
    Element* getFaceRoot();
}