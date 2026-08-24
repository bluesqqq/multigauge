# Host integration

This repository is a portable library. The host provides implementations of
`GraphicsContext`, `FileSystem`, and `Time`; `Logger` is optional.

Construct and initialize `mg::Runtime`, register a render surface with `Runtime::addContext(...)`,
and call `Runtime::frame()` from the host frame loop. Keep the registered graphics
context dimensions current when its render surface changes size.

`RuntimeConfig::dataRoot` selects the storage root used by the public package API.
See [storage.md](./storage.md) for its file contract.
