# Portable Core Guide

This repository is the portable multigauge core library.

## Portability and public contracts

- Build as C++20. Keep headers and sources platform-independent: no Arduino/ESP32, Emscripten/DOM, OS filesystem, or concrete graphics-backend APIs.
- Express platform needs through the interfaces in `include/multigauge/graphics/GraphicsContext.h` and `include/multigauge/io/`; initialization flows through an explicit `mg::Runtime` instance.
- `include/multigauge/` is public; `src/` is implementation-only. Preserve source compatibility unless a break is intentional, and document non-obvious public contracts in the headers themselves.
- The core uses value types plus `std::unique_ptr` for exclusive ownership of screens, gauge elements, faces, and colors, and `std::shared_ptr` for shared editor-history state. Treat raw pointers/references as borrowed unless the API explicitly says otherwise. Preserve documented lifetime rules such as `Value`'s borrowed `std::string_view` identity/name and `UnitType` reference.
- Dynamic allocation is supported and common; there is no repository rule banning it. Still avoid unnecessary allocation/copying in per-frame `update`/`draw`, image decode, and other hot paths.
- Core CMake does not override exception or RTTI modes. Do not change those modes or make shared code depend on target-specific exception/RTTI behavior without verifying the targets this repo actually builds.
- `MULTIGAUGE_BUILD_EDITOR=OFF` excludes editor implementation and editor screens, and also disables editor-facing property metadata. Keep a no-editor build free of editor dependencies; port builds that do not expose editing should use the equivalent `MG_BUILD_EDITOR=0` define.

## Serialization and registries

- `MG_PROP` keys and `MG_TYPE_ID` strings are serialized and editor-facing. Keep element/color registries, property codecs/metadata, `docs/schemas/gauge.schema.json`, examples, and JS editor widgets consistent.
- Built-in IDs in `src/value/Value.cpp`, unit-type lookup names and unit ordering in `src/value/UnitType.cpp`, and `ValueRef` string encoding are compatibility-sensitive.
- Use RapidJSON through the existing codecs/helpers and check decode failures. Do not silently rename or discard serialized fields.

## Repository Layout

- `include/` holds the public API and should stay portable.
- `src/` holds implementation code only.
- `tests/` contains the doctest unit suite and its CMake wiring.
- `cmake/` is for helper modules only.
- The build fetches its third-party dependencies during configure and should not depend on pre-populated local vendor folders or parent-directory paths.

## Build and tests

From the repository root, configure, build, and run the doctest suite. With a Visual Studio generator, `ctest` needs the active configuration:

```powershell
cmake -S . -B build
cmake --build build --config Debug --target multigauge_core_tests
ctest --test-dir build -C Debug --output-on-failure
```

To verify the lean runtime configuration, configure with `-DMULTIGAUGE_BUILD_EDITOR=OFF`; its test target intentionally omits editor-specific test files.

Tests live in `tests/` and are registered through CTest. Add focused doctest cases and list new files in `tests/CMakeLists.txt`.

## Dependency Fetching

- The root `CMakeLists.txt` fetches pinned third-party sources during configure with `FetchContent`.
- Do not commit third-party source trees into this repository.
- Keep dependency versions pinned and update them intentionally when changing the fetch URLs or tags.
