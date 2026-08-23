#pragma once

#include <memory>
#include <string>

#include <multigauge/runtime/AssetManager.h>
#include <multigauge/screens/Screen.h>
#include <multigauge/gauge/GaugeFace.h>

namespace mg {

class GaugeScreen : public Screen {
    private:
        std::unique_ptr<::mg::gauge::GaugeFace> face = nullptr;
        std::string packageId;

    public:
        explicit GaugeScreen();

        void setFace(std::unique_ptr<::mg::gauge::GaugeFace> face, std::string packageId = {});

        void onShow(RuntimeContext& ctx) override;
        void onHide(RuntimeContext& ctx) override;
        void update(RuntimeContext& ctx, std::chrono::microseconds delta) override;
        void draw(RuntimeContext& ctx, graphics::Graphics& g) override;
};

}
