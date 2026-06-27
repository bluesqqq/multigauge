<p align="center">
  <a href="https://www.multi-gauge.com/">
    <picture>
      <source media="(prefers-color-scheme: dark)" srcset="./assets/multigauge-wordmark-light.png">
      <source media="(prefers-color-scheme: light)" srcset="./assets/multigauge-wordmark-dark.png">
      <img src="./assets/multigauge-wordmark-dark.png" alt="Multigauge" width="250">
    </picture>
  </a>
</p>

<p align="center">
  <em> <a href="https://www.multi-gauge.com/"><b>Multigauge</b></a> is a platform-agnostic <b>automotive gauge rendering engine</b> for building highly customizable, data-driven digital dashboards. It aims to make custom instrumentation both <b>powerful</b> and <b>aesthetically pleasing.</b></em>
</p>

## Repository Structure

```
multigauge/
├─ core/ # Shared platform-agnostic C++ engine
├─ ports/ # Platform-specific implementations
│ ├─ esp32/ # ESP32 embedded target
│ └─ web/ # WebAssembly/web target
├─ apps/ # Applications built on top of Multigauge
├─ docs/ # Project documentation
```

## Core

The core engine is designed to be portable and independent of any specific display, OS, or hardware platform.

```core/``` contains the shared C++ code for:

- JSON-defined gauge layout
- Gauge elements and styling
- Layout calculation
- Rendering abstractions
- Editor-facing metadata
- Serialization support
- Asset loading interfaces
- Timing, logging, and platform service interfaces

## Ports

### Current Ports

- `ports/esp32/` - ESP32 embedded implementation
- `ports/web/` - WebAssembly/web implementation

### Planned Ports

Roughly in order of priority:

- Linux
- STM32
- Arduino
- Windows

## Demos

I should put some demos here...

## Status

`multigauge` and its targets are actively under development. The core architecture is in
place, but breaking changes may still occur as the project evolves.

APIs, file formats, target integrations, and editor-facing metadata are likely to change.

## License

This project is free to use for personal, educational, and non-commercial purposes.

You may not use this code in any product or service that is sold or monetized
without permission.

If you're unsure whether your use case is allowed, feel free to reach out.
