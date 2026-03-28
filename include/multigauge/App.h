#pragma once

#include <cstdint>
#include <memory>

#include <multigauge/gauge/GaugeFace.h>

class AssetManager;
class Graphics;
class GraphicsContext;

class GaugeView {
    private:
        GraphicsContext* context = nullptr;
        std::unique_ptr<Graphics> graphics;
        std::unique_ptr<AssetManager> assets;
        GaugeFace face;

    public:
        explicit GaugeView(GraphicsContext& context);
        ~GaugeView();

        GaugeView(const GaugeView&) = delete;
        GaugeView& operator=(const GaugeView&) = delete;
        GaugeView(GaugeView&&) = delete;
        GaugeView& operator=(GaugeView&&) = delete;

        bool load(const char* gaugePath);
        void frame(uint64_t deltaUs);

        GraphicsContext& getContext();
        const GraphicsContext& getContext() const;

        Graphics& getGraphics();
        const Graphics& getGraphics() const;

        GaugeFace& getGaugeFace();
        const GaugeFace& getGaugeFace() const;
};

namespace mg {
    GaugeView& createView(GraphicsContext& context, const char* gaugePath = nullptr);
    bool removeView(GaugeView& view);
    void clearViews();
    GaugeView* getPrimaryView();

    bool init(const char* gaugePath);
    void frame();
    GaugeFace& getGaugeFace();
}
