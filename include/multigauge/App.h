#pragma once

#include <multigauge/gauge/GaugeFace.h>

namespace mg {
    bool init(const char* gaugePath);
    void frame();
    GaugeFace& getGaugeFace();
}