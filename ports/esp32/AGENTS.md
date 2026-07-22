# ESP32 Port Guide

This file refines the repository guide for `ports/esp32/`.

## Target and boundaries

- The only configured PlatformIO environment is `esp32-s3-devkitc-1`. It uses the Arduino framework, LittleFS, and GNU C++20 through the remote stable `platform-espressif32` package.
- The current effective board profile is ESP32-S3-DevKitC-1 N8: 8 MB flash, 320 KB RAM, and no PSRAM. The hardware implementation targets a 240x240 GC9A01 display and CST816S touch controller; pin/bus configuration lives in `src/platform/LGFX_definition.h` and is documented in `README.md`.
- Keep Arduino, Wi-Fi, LittleFS, LovyanGFX, ESP heap, display/touch, and sensor code in this port. Changes needed by multiple ports belong behind a core interface, not in core as conditional ESP32 code.
- `src/platform/` implements the portable graphics, filesystem, time, and logger services. Preserve `setup`/`loop` startup ordering: initialize core services, initialize the display context, register it, then drive `mg::frame()` and web processing.

## Memory, I/O, and generated dependencies

- Allocation is allowed, but this board has no PSRAM. Review peak heap and fragmentation for image decoding, sprites, JSON/package imports, upload buffering, containers, and strings; avoid new per-frame allocation and unbounded buffers.
- Keep flash/LittleFS usage and firmware size visible in build output. Package upload/management must retain the root storage-safety rules and must not perform arbitrary client-selected filesystem access.
- `bootstrap-deps.py` runs before PlatformIO and invokes CMake bootstrap. It creates `lib/` links/copies and repository `.deps/`; `.pio/`, `lib/`, and generated `library.json` files below `lib/` are not source. Change `cmake/Dependencies.cmake`, the root dependency definitions, or `platformio-overrides/` instead.
- `platformio.ini` removes GNU C++11 and adds GNU C++20, but does not declare a project exception/RTTI policy. Preserve the effective toolchain behavior unless a deliberate cross-target change is verified.

## Build and hardware verification

From the repository root, use the same firmware build as CI:

```powershell
pio run -d ports/esp32 -e esp32-s3-devkitc-1
```

Flashing requires a connected device:

```powershell
pio run -d ports/esp32 -e esp32-s3-devkitc-1 -t upload
pio run -d ports/esp32 -e esp32-s3-devkitc-1 -t uploadfs
```

There is currently no tracked `test/` suite despite `test_build_src = true`; do not claim `pio test` coverage. Hardware-affecting work requires targeted physical checks of the affected display/touch/pins, LittleFS persistence, boot/serial logs, Wi-Fi access point/web upload flow, or sensors. Report which checks could not be performed without hardware.
