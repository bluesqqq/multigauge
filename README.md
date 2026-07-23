<p align="center">
  <picture>
    <source media="(prefers-color-scheme: dark)" srcset="assets/multigauge-wordmark-light.png">
    <source media="(prefers-color-scheme: light)" srcset="assets/multigauge-wordmark-dark.png">
    <img src="assets/multigauge-wordmark-dark.png" alt="Multigauge" width="280">
  </picture>
</p>

<p align="center">
  <em>
    <a href="https://www.multi-gauge.com/"><b>Multigauge</b></a> is a platform-agnostic
    <b>automotive gauge rendering engine</b> for building highly customizable, data-driven digital dashboards.
    It aims to make custom instrumentation both <b>powerful</b> and <b>aesthetically pleasing</b>.
  </em>
</p>

It provides:

- Gauge and dashboard models
- Flexible layout
- JSON serialization
- Telemetry values and units
- Asset and package handling
- Platform-agnostic rendering and system interfaces

## Layout

```text
include/multigauge/     Public C++ API
src/                    Core implementation
tests/                  doctest unit tests
CMakeLists.txt          Root CMake build
library.json            PlatformIO library metadata
```

## Requirements

- CMake 3.16 or newer.
- A C and C++ toolchain with C++20 support.
- Network access when configuring for the first time to fetch dependencies.

## Build

From the repository root:

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

## Test

Run the registered doctest suite through CTest:

```sh
ctest --test-dir build -C Debug --output-on-failure
```

## Status

> [!WARNING]
> Multigauge is under active development. Public APIs and serialized formats may change before the first stable release.

## Contributing

Contributions that improve the portable core, tests, documentation, and host-neutral extension points are welcome. Before opening a pull request:

1. Keep public headers platform-independent.
2. Add or update focused tests for behavioral changes.
3. Run the configure, build, and test commands shown above.

## License

Multigauge is licensed under the [Mozilla Public License 2.0](LICENSE).