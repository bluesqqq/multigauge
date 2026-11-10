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

`FileSystem::create(path)` creates missing parent directories and asks
the backend to create a new writable file atomically. Its result distinguishes
an occupied path from other failures. Recording destinations must implement
`createImpl()` using an exclusive creation primitive supplied by the
backend. Backends that lack this operation report a creation failure.

`readBytes` and `readText` require a `maxBytes` content limit. They reject oversized
files before allocating, enforce the limit while reading growing files, and leave
the output unchanged on failure. Zero permits only empty files. The limit does
not cap total RAM usage (text conversion and caller-owned output can coexist).
Use streams for recordings and other large files. `json::readJsonFile` defaults
to 256 KiB and accepts an explicit override; image reads are limited to 64 KiB.

Construct and initialize `mg::Runtime`, register a render surface with `runtime.contexts().add(...)`,
and call `Runtime::frame()` from the host frame loop. Keep the registered graphics
context dimensions current when its render surface changes size.

After `Runtime::init()` succeeds, `runtime.recordings()` returns a borrowed
[recording manager](../include/multigauge/runtime/RecordingManager.h) valid until
`Runtime::shutdown()`. It uses the runtime filesystem by default. A host can set
`RuntimeConfig::recordingFileSystem` to a separate destination, such as an SD
backend, and keep it alive for the lifetime of the runtime. The host initializes
that separate destination before recording. `Runtime::init()` only initializes
the primary filesystem, so missing separate recording media affects only logging.

The host chooses the channels, applies the Fast, Normal, and Slow sampling
presets, and decides when to omit unchanged values. On the playback timeline, a
channel has no value until its first sample, then holds its last recorded value
until its next update. The host supplies the RTC start time when it has a trusted
UTC clock. Samples and the final duration use monotonic elapsed time, so clock
adjustments leave the timeline intact. The duration also preserves time after
the last update.

On a real-time host, use a bounded handoff from the sampler to a storage task that
calls the manager with ordered batches. The frame loop, sensor acquisition, and
controls should continue without waiting for recording I/O. The storage task can
call `flush()` approximately every 1–5 seconds to limit buffered data and publish
a copy of the manager's status for other tasks. To finish a recording,
the storage task drains its pending batches and checks the result of
`stop(duration)` before the host shuts down `Runtime` or unmounts the destination.
Shutting down with an active recording closes its file without a final duration.
The [binary recording format](./recording-format.md) supports recovering complete
blocks before a partial tail. Listing and playback belong to the reader and its
consumers.

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
