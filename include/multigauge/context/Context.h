#pragma once

#include <chrono>
#include <memory>
#include <string>

#include <multigauge/graphics/Graphics.h>
#include <multigauge/runtime/AssetManager.h>

namespace mg {

class Screen;
using OwnedScreen = std::unique_ptr<Screen>;

namespace context {

/// @brief Owns the rendering state and active screen for one graphics context.
class Context {
public:
    //----------[ CTOR + DTOR ]----------//

    Context(
        graphics::GraphicsContext& graphicsContext,
        io::FileSystem& fs,
        std::string dataRoot,
        const graphics::UserPalette& userPalette
    );

    ~Context();

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;
    Context(Context&&) noexcept = default;
    Context& operator=(Context&&) noexcept = default;


    //----------[ GRAPHICS ]----------//

    graphics::GraphicsContext& getGraphicsContext();

    const graphics::GraphicsContext& getGraphicsContext() const;

    graphics::Graphics& getGraphics();

    const graphics::Graphics& getGraphics() const;

    void setBackgroundColor(graphics::rgba color);

    graphics::rgba getBackgroundColor() const;

    //----------[ ASSET MANAGER ]----------//

    AssetManager& getAssetManager();

    const AssetManager& getAssetManager() const;

    //----------[ SCREEN ]----------//

    void clearScreen();

    bool setScreen(OwnedScreen screen);

    Screen* getScreen();

    const Screen* getScreen() const;

    //----------[ LIFECYCLE ]----------//

    void frame(std::chrono::microseconds delta, std::chrono::microseconds elapsed);

private:
    graphics::GraphicsContext* graphicsContext_;
    graphics::Graphics graphics_;
    AssetManager assets_;
    OwnedScreen screen_;
    graphics::rgba backgroundColor_ = graphics::rgba(0, 0, 0, 255);
    graphics::ColorFrame colorFrame_;
    const graphics::UserPalette* userPalette_;

};

} // namespace context

} // namespace mg
