# Portable Core Guide

This repository is the portable multigauge core library.

## Portability and public contracts

- Build as C++20. Keep headers and sources platform-independent: no Arduino/ESP32, Emscripten/DOM, OS filesystem, or concrete graphics-backend APIs.
- Express platform needs through the interfaces in `include/multigauge/graphics/GraphicsContext.h` and `include/multigauge/io/`; initialization flows through `mg::init`. Update `docs/porting.md` when this contract changes.
- `include/multigauge/` is public; `src/` is implementation-only. Preserve source compatibility unless a break is intentional, and document non-obvious public contracts using `docs/style/doxygen.md`.
- The core uses value types plus `std::unique_ptr` for exclusive ownership of screens, gauge elements, faces, and colors, and `std::shared_ptr` for shared editor-history state. Treat raw pointers/references as borrowed unless the API explicitly says otherwise. Preserve documented lifetime rules such as `Value`'s borrowed `std::string_view` identity/name and `UnitType` reference.
- Dynamic allocation is supported and common; there is no repository rule banning it. Still avoid unnecessary allocation/copying in per-frame `update`/`draw`, image decode, and other hot paths because this code also runs on a no-PSRAM ESP32 target.
- Core CMake does not override exception or RTTI modes. Do not change those modes or make shared code depend on target-specific exception/RTTI behavior without verifying native, web, and ESP32 builds.

## Serialization and registries

- `MG_PROP` keys and `MG_TYPE_ID` strings are serialized and editor-facing. Keep element/color registries, property codecs/metadata, `docs/schemas/gauge.schema.json`, examples, and JS editor widgets consistent.
- Built-in IDs in `src/value/Value.cpp`, unit-type lookup names and unit ordering in `src/value/UnitType.cpp`, and `ValueRef` string encoding are compatibility-sensitive.
- Package-manager behavior must remain aligned with `docs/storage.md` and the package, manifest, and library schemas. Preserve validation-before-write, internally derived safe paths, import/export shape and order, and the distinction between authoritative manifests/faces and derived `library.json`.
- Use RapidJSON through the existing codecs/helpers and check decode failures. Do not silently rename or discard serialized fields.

## Build and tests

From the repository root, configure, build, and run the doctest suite:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug --target multigauge_core_tests
ctest --test-dir build -C Debug --output-on-failure
```

Tests live in `tests/` and are registered through CTest. Add focused doctest cases and list new files in `tests/CMakeLists.txt`. Cross-platform integration builds run from the private monorepo.
