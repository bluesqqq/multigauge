# Host integration

This repository is a portable library. The host provides implementations of
`GraphicsContext`, `FileSystem`, and `Time`; `Logger` is optional.

`FileSystem::open(path, FileMode)` provides streaming access through an
exclusively owned `io::File`. Backends implement `openImpl`, `listFiles`, and
directory/file queries.
Whole-file helpers use streams rather than backend overrides. Write and append
opens create parent directories. Read opens never create files. Write truncates;
append always writes at EOF, including after a seek. Handles support bounded
reads, full writes, absolute seeks within the file, position queries (`tell`),
flush, and idempotent close.
EOF is a successful read of zero bytes; failures may transfer a prefix. Destroying
a handle closes it, but callers should explicitly flush/close when they need to
observe failures. Backends cannot necessarily detect all device errors or guarantee
power-loss durability. Keep the filesystem alive until all its handles are closed,
and serialize access to handles on the host. `listFiles` and `listDirectories`
return immediate child names rather than full paths.

`readBytes` and `readText` require a `maxBytes` content limit. They reject oversized
files before allocating, enforce the limit while reading growing files, and leave
the output unchanged on failure. Zero permits only empty files. The limit does
not cap total RAM usage (text conversion and caller-owned output can coexist).
Use streams for recordings and other large files. `json::readJsonFile` defaults
to 256 KiB and accepts an explicit override; image reads are limited to 64 KiB.

Construct and initialize `mg::Runtime`, register a render surface with `runtime.contexts().add(...)`,
and call `Runtime::frame()` from the host frame loop. Keep the registered graphics
context dimensions current when its render surface changes size.

`RuntimeConfig::dataRoot` selects the storage root used by package, settings, and
sensor persistence. See [storage.md](./storage.md) for its file contract. Hosts
can read `runtime.settings().activeFace()` after creating a graphics context and
restore it with `runtime.navigation().showGauge(...)`.

To support physical controls, persist user assignments with
`runtime.controls().bind(...)` and `save()`, then call
`trigger(contextId, portIndex)` when an input activates. On a gauge screen, `NEXT`
and `PREVIOUS` cycle the faces in
the current package's stored order. `trigger` returns a `control::Result`; `SELECT`
returns `OpenMenu` until a menu screen is available. Other screens can handle
controls through their optional `Screen::onControl(action)` override. Bindings are
local to one runtime; they do not identify or route to another gauge.

The host owns its physical pin table and maps its pins to zero-based control slot
indices. Core has no knowledge of GPIOs or user-facing port labels. Targets can
set their fixed storage capacity with `MG_CONTROL_MAX_PORTS` (the ESP32 target
currently sets it to zero until CONTROL pins are assigned); the CMake equivalent
is `MULTIGAUGE_CONTROL_MAX_PORTS`.
