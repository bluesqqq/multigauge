#pragma once

#include <chrono>

#include <multigauge/control/Action.h>
#include <multigauge/control/Result.h>

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

    /// @brief Handles a navigation action from a bound control port.
    /// @return The result that the context manager should apply.
    virtual control::Result onControl(const control::Action&) {
        return control::Result::ignored();
    }

    virtual void update(context::Context& context, std::chrono::microseconds delta) = 0;
    virtual void draw(context::Context& context, graphics::Graphics& g) = 0;
};

}
