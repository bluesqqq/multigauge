#pragma once

#include <memory>
#include <string>

#include <multigauge/AssetManager.h>
#include <multigauge/screens/Screen.h>
#include <multigauge/gauge/GaugeFace.h>

namespace mg {

class GaugeScreen : public Screen {
    private:
        std::unique_ptr<AssetManager> assets;
        std::unique_ptr<GaugeFace> face;
        std::string gaugePath;
        RuntimeContext* context = nullptr;
        bool loaded = false;

        bool reload();

    public:
        explicit GaugeScreen(const char* gaugePath = nullptr);

        bool load(const char* gaugePath);
        void unload();

        void onShow(RuntimeContext& ctx) override;
        void onHide(RuntimeContext& ctx) override;
        void update(RuntimeContext& ctx, uint64_t deltaUs) override;
        void draw(RuntimeContext& ctx, Graphics& g) override;
};

}
