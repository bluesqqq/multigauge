#pragma once

#include <chrono>

namespace mg {

namespace context { class Context; }
namespace graphics { class Graphics; }

class Screen {
public:
    //----------[ CTOR ]----------//

    virtual ~Screen() = default;

    //----------[ LIFECYCLE ]----------//

    virtual void onShow(context::Context& context) {};
    virtual void onHide(context::Context& context) {};

    virtual void update(context::Context& context, std::chrono::microseconds delta) = 0;
    virtual void draw(context::Context& context, graphics::Graphics& g) = 0;
};

}
