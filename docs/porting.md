# Host integration

This repository is a portable library. The host provides implementations of
`GraphicsContext`, `FileSystem`, and `Time`; `Logger` is optional.

Call `mg::init(...)` once, register a render surface with `mg::addContext(...)`,
and call `mg::frame()` from the host frame loop. Keep the registered graphics
context dimensions current when its render surface changes size.

`AppConfig::dataRoot` selects the storage root used by the public package API.
See [storage.md](./storage.md) for its file contract.
