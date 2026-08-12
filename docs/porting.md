# Integrating multigauge

The library is platform-independent. A host application supplies implementations of:

- [`GraphicsContext`](../include/multigauge/graphics/GraphicsContext.h)
- [`FileSystem`](../include/multigauge/io/FileSystem.h)
- [`Time`](../include/multigauge/io/Time.h)
- optionally, [`Logger`](../include/multigauge/io/Logger.h)

At startup, call `mg::init(...)`, then register each render surface with
`mg::addContext(...)`. Call `mg::frame()` once per host frame. The supplied
`GraphicsContext` dimensions must stay current, including after a resize.

`AppConfig::dataRoot` selects the library's storage root. See
[storage.md](./storage.md) for the files managed below it.
