#pragma once

#include <multigauge/runtime/RuntimeContext.h>

namespace mg {

class Screen {
    public:
        virtual ~Screen() = default;

        virtual void onShow(RuntimeContext& context) {};
        virtual void onHide(RuntimeContext& context) {};

        virtual void update(RuntimeContext& context, uint64_t deltaUs) = 0;
        virtual void draw(RuntimeContext& context, graphics::Graphics& g) = 0;
};

}
