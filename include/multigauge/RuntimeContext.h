#pragma once

#include <cstdint>
#include <memory>

#include <multigauge/graphics/Graphics.h>

namespace mg {

using ContextId = uint32_t;

class Screen;
using OwnedScreen = std::unique_ptr<Screen>;

class RuntimeContext {
    private:
        GraphicsContext* native = nullptr;
        Graphics graphics;
        OwnedScreen activeScreen;
        ContextId id = 0;
        rgba backgroundColor = rgba(0, 0, 0, 255);

    public:
        explicit RuntimeContext(GraphicsContext& graphicsContext);
        ~RuntimeContext();

        RuntimeContext(const RuntimeContext&) = delete;
        RuntimeContext& operator=(const RuntimeContext&) = delete;
        RuntimeContext(RuntimeContext&&) noexcept = default;
        RuntimeContext& operator=(RuntimeContext&&) noexcept = default;

        void setId(ContextId contextId);
        ContextId getId() const;

        GraphicsContext& getGraphicsContext();
        const GraphicsContext& getGraphicsContext() const;

        Graphics& getGraphics();
        const Graphics& getGraphics() const;

        void setBackgroundColor(rgba color);
        rgba getBackgroundColor() const;

        void clearScreen();
        bool setScreen(OwnedScreen screen);

        Screen* getScreen();
        const Screen* getScreen() const;

        void frame(uint64_t deltaUs);
};

}
