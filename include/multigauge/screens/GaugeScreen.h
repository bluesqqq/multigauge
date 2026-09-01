#pragma once

#include <memory>
#include <string>

#include <multigauge/runtime/AssetManager.h>
#include <multigauge/screens/Screen.h>
#include <multigauge/gauge/GaugeFace.h>

namespace mg {

class GaugeScreen : public Screen {
public:
    //----------[ CTOR ]----------//

    explicit GaugeScreen();

    //----------[ FACE ]----------//

    void setFace(std::unique_ptr<::mg::gauge::GaugeFace> face, std::string packageId = {});

    //----------[ LIFECYCLE ]----------//

    void onShow(context::Context& ctx) override;
    void onHide(context::Context& ctx) override;

    void update(context::Context& ctx, std::chrono::microseconds delta) override;
    void draw(context::Context& ctx, graphics::Graphics& g) override;
    
private:
    std::unique_ptr<::mg::gauge::GaugeFace> face = nullptr;
    std::string packageId;

};

}
