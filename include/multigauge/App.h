#pragma once

#include <multigauge/gauge/GaugeFace.h>

class GaugeView;
class GraphicsContext;

namespace mg {
    GaugeView& createView(GraphicsContext& context, const char* gaugePath = nullptr);
    bool removeView(GaugeView& view);
    void clearViews();
    GaugeView* getPrimaryView();

    bool init(const char* gaugePath);
    void frame();
    GaugeFace& getGaugeFace();
}
