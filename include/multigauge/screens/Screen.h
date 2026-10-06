#pragma once

#include <chrono>

namespace mg {

namespace context { class Context; }
namespace control { struct Action; }
namespace graphics { class Graphics; }

class Screen {
public:
    //----------[ CTOR ]----------//

    virtual ~Screen() = default;

    //----------[ LIFECYCLE ]----------//

    virtual void onShow(context::Context& context) {};
    virtual void onHide(context::Context& context) {};

    /// @brief Handles a navigation action from a bound control port.
    /// @return True when the screen consumed the action.
    virtual bool onControl(const control::Action&) { return false; }

    virtual void update(context::Context& context, std::chrono::microseconds delta) = 0;
    virtual void draw(context::Context& context, graphics::Graphics& g) = 0;
};

}
