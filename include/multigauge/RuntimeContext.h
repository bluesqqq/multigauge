#pragma once

#include <cstdint>
#include <memory>

#include <multigauge/graphics/Graphics.h>
#include <multigauge/AssetManager.h>

namespace mg {

using ContextId = uint32_t;

class Screen;
using OwnedScreen = std::unique_ptr<Screen>;

class RuntimeContext {
    private:
        graphics::GraphicsContext* context;
        graphics::Graphics graphics;

        AssetManager assets;

        OwnedScreen screen;
        graphics::rgba backgroundColor = graphics::rgba(0, 0, 0, 255);

    public:
        explicit RuntimeContext(graphics::GraphicsContext& graphicsContext, io::FileSystem& fs);
        ~RuntimeContext();

        RuntimeContext(const RuntimeContext&) = delete;
        RuntimeContext& operator=(const RuntimeContext&) = delete;
        RuntimeContext(RuntimeContext&&) noexcept = default;
        RuntimeContext& operator=(RuntimeContext&&) noexcept = default;

        graphics::GraphicsContext& getGraphicsContext();
        const graphics::GraphicsContext& getGraphicsContext() const;

        graphics::Graphics& getGraphics();
        const graphics::Graphics& getGraphics() const;

        AssetManager& getAssetManager();
        const AssetManager& getAssetManager() const;

        void setBackgroundColor(graphics::rgba color);
        graphics::rgba getBackgroundColor() const;

        void clearScreen();
        bool setScreen(OwnedScreen screen);

        Screen* getScreen();
        const Screen* getScreen() const;

        void frame(uint64_t deltaUs);
};

}
