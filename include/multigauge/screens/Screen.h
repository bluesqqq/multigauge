#pragma once

#include <chrono>

#include <multigauge/runtime/RuntimeContext.h>

namespace mg {

class Screen {
    public:
        virtual ~Screen() = default;

        virtual void onShow(RuntimeContext& context) {};
        virtual void onHide(RuntimeContext& context) {};

        virtual void update(RuntimeContext& context, std::chrono::microseconds delta) = 0;
        virtual void draw(RuntimeContext& context, graphics::Graphics& g) = 0;
};

}
