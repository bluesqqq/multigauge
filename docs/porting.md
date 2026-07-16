# Porting multigauge-core

This guide covers the minimum pieces required to bring `multigauge-core` to a new target.

## Overview

`multigauge-core` is designed to stay platform-agnostic. A target implementation is responsible for supplying the platform services the core runtime depends on:

- [`GraphicsContext`](../core/include/multigauge/graphics/GraphicsContext.h)
- [`FileSystem`](../core/include/multigauge/io/FileSystem.h)
- [`Time`](../core/include/multigauge/io/Time.h)
- [`Logger`](../core/include/multigauge/io/Logger.h) (optional)

Those services are registered through `mg::init(...)`, which must be called before using the core API.

## Porting Checklist

- Implement a target-specific [`GraphicsContext`](../core/include/multigauge/graphics/GraphicsContext.h)
- Implement a target-specific [`FileSystem`](../core/include/multigauge/io/FileSystem.h)
- Implement a target-specific [`Time`](../core/include/multigauge/io/Time.h)
- Optionally implement a [`Logger`](../core/include/multigauge/io/Logger.h)
- Call `mg::init(...)` once during startup
- Register a graphics context with `mg::addContext(...)`
- Drive rendering by calling `mg::frame()` from your target's main loop

## Required Services

### GraphicsContext

[`GraphicsContext`](../core/include/multigauge/graphics/GraphicsContext.h) is the largest part of a new port. It provides the low-level drawing primitives used by the higher-level `Graphics` wrapper.

At a minimum, your implementation must support:

- frame lifecycle hooks: `init()`, `beginFrame()`, `endFrame()`
- screen sizing through `resize()`, `width()`, and `height()`
- primitive drawing for pixels, lines, rectangles, circles, ellipses, rings, arcs, and triangles
- text measurement and drawing
- native image creation and drawing
- clipping

In practice, most ports map these calls onto an existing graphics backend or hardware display library.

### FileSystem

[`FileSystem`](../core/include/multigauge/io/FileSystem.h) is used to load gauge definitions and assets.

Current expectations:

- `readBytes(...)` must read an entire file into memory
- `exists(...)` and `size(...)` are used by the asset loader
- write and directory helpers should behave normally even if your target only uses a subset of them today

Gauge JSON is loaded from files in the filesystem you provide to `mg::init(...)`.

Image asset loading currently prefixes paths with `/assets/images/`, so image references in gauge documents are expected to resolve under that directory on the target filesystem.

### Time

[`Time`](../core/include/multigauge/io/Time.h) is used for frame timing and animation updates.

Your implementation must provide:

- `getMillis()`
- `getMicros()`

`mg::frame()` currently uses microsecond timing, so `getMicros()` should be monotonic and reasonably high-resolution.

### Logger

[`Logger`](../core/include/multigauge/io/Logger.h) is optional but strongly recommended while bringing up a new port.

If no logger is supplied, the engine can still run, but you lose useful diagnostics during file loading, image decoding, and runtime debugging.

## Startup Flow

The expected startup sequence is:

1. Construct your concrete platform services
2. Call `mg::init(fs, time, config, logger)`
3. Register a graphics context with `mg::addContext(graphicsContext)`
4. Load a gauge or editor screen
5. Call `mg::frame()` every frame

## Minimal Skeleton

```cpp
#include <multigauge/App.h>
#include <multigauge/graphics/GraphicsContext.h>
#include <multigauge/io/FileSystem.h>
#include <multigauge/io/Logger.h>
#include <multigauge/io/Time.h>

class MyGraphicsContext : public mg::graphics::GraphicsContext {
    // Implement drawing backend here
};

class MyFileSystem : public mg::io::FileSystem {
    // Implement file access here
};

class MyTime : public mg::io::Time {
    // Implement timing here
};

class MyLogger : public mg::io::Logger {
    // Implement logging here
};

int main() {
    MyGraphicsContext gfx;
    MyFileSystem fs;
    MyTime time;
    MyLogger logger;

    if (!mg::init(fs, time, mg::AppConfig{}, &logger)) {
        return 1;
    }

    auto contextId = mg::addContext(gfx);
    (void)contextId;

    while (true) {
        mg::frame();
    }
}
```

Adjust the file paths and application loop to match your target environment.

## Notes and Pitfalls

- Image decoding is handled by the core, but the final `Image` object is created by your `GraphicsContext`
- Gauge layout depends on the current render target size, so `GraphicsContext::width()` and `height()` must stay accurate
- If your target supports resizing, make sure `resize()` updates the stored dimensions correctly
- The current core API assumes assets can be loaded synchronously
- Some parts of the engine are still evolving, so expect minor integration changes while the project is under active development

## Related Files

- [`App`](../core/include/multigauge/App.h)
- [`GraphicsContext`](../core/include/multigauge/graphics/GraphicsContext.h)
- [`FileSystem`](../core/include/multigauge/io/FileSystem.h)
- [`Time`](../core/include/multigauge/io/Time.h)
- [`Logger`](../core/include/multigauge/io/Logger.h)
