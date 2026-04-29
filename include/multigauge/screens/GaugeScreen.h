#pragma once

#include <memory>
#include <string>

#include <multigauge/AssetManager.h>
#include <multigauge/screens/Screen.h>
#include <multigauge/gauge/GaugeFace.h>

namespace mg {

class GaugeScreen : public Screen {
    private:
        std::unique_ptr<::mg::gauge::GaugeFace> face = nullptr;

    public:
        explicit GaugeScreen();

        void onShow(RuntimeContext& ctx) override;
        void onHide(RuntimeContext& ctx) override;
        void update(RuntimeContext& ctx, uint64_t deltaUs) override;
        void draw(RuntimeContext& ctx, graphics::Graphics& g) override;
};

}
