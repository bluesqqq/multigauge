# Host integration

This repository is a portable library. The host provides implementations of
`GraphicsContext`, `FileSystem`, and `Time`; `Logger` is optional.

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
sets it to four); the CMake equivalent is `MULTIGAUGE_CONTROL_MAX_PORTS`.
