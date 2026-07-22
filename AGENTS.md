# Multigauge Agent Guide

## Purpose and map

Multigauge is a monorepo for a portable C++20 automotive gauge renderer/editor and the targets and applications built around it.

- `core/`: platform-independent engine, editor, runtime, public headers, and native tests.
- `ports/esp32/`: Arduino/PlatformIO firmware and ESP32 hardware services.
- `ports/web/`: Emscripten/WebAssembly runtime, C bindings, browser loader, and examples.
- `apps/website/`: Flask/Jinja public site, workshop/editor UI, database models, and static assets.
- `docs/`: porting, storage, element, release, public-header style, and canonical JSON-schema documentation.
- `cmake/`: shared dependency versions/bootstrap; `.deps/` is generated dependency state.
- `scripts/`: web-bundle publishing and firmware-manifest generation.
- `.github/workflows/`: CI and the repository-wide release pipeline; `VERSION` is the single release version.

Read the nearest nested `AGENTS.md` before changing a component. Use [README.md](README.md) for the overview, [docs/porting.md](docs/porting.md) for the platform contract, [docs/storage.md](docs/storage.md) for installed/package data, [docs/style/doxygen.md](docs/style/doxygen.md) for public C++ docs, and [docs/releasing.md](docs/releasing.md) for releases.

## Boundaries and compatibility

- Keep `core/` free of Arduino, ESP32, Emscripten, browser, and vendor-backend dependencies. Ports implement `GraphicsContext`, `FileSystem`, `Time`, and optional `Logger`, then register them through `mg::init`.
- Put hardware/browser integration in the relevant port. The Flask app consumes published web artifacts; it is not a second implementation of the renderer.
- Treat headers under `core/include/multigauge/` as public API. Make signature, ownership, lifetime, and behavior changes deliberately and update callers and contract documentation together.
- Treat serialized names as compatibility surfaces: JSON property keys, `MG_TYPE_ID` values, built-in value IDs, unit-type/unit identifiers and ordering, editor result shapes, package/storage IDs, schema fields, and exported `mg_*` WebAssembly functions. Update code, schemas under `docs/schemas/`, examples, and consumers together when a deliberate format change is required.
- `docs/storage.md` is authoritative for package import/export and installed storage. Derive paths internally; never trust uploaded IDs, paths, or filenames as storage locations.
- Shared C++ must remain viable on both native/WebAssembly builds and the ESP32 target. Review code size, peak memory, hot-path allocation, synchronous I/O, and unavailable platform facilities.

## Toolchains and generated content

- Native core and WebAssembly targets require C++20. The root superbuild and component projects require CMake 3.16 or newer; web presets require CMake 3.25 and Ninja.
- ESP32 uses PlatformIO, the Arduino framework, GNU C++20, and the `esp32-s3-devkitc-1` environment. See its nested guide for the current hardware profile.
- WebAssembly requires emsdk/Emscripten. CI installs the latest emsdk rather than pinning a version.
- Dependency bootstrap populates `.deps/` and `ports/esp32/lib/`. Builds populate `build/`, `ports/esp32/.pio/`, `ports/web/dist/`, and the published `apps/website/static/multigauge-web/`. Do not hand-edit or commit generated/ignored output.
- The repository has no configured repository-wide formatter, linter, or static-analysis command. Match nearby style; do not present an ad hoc formatter as project policy.

## Build and verification from the repository root

Use the CI-equivalent native workflow for shared/core changes:

```powershell
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DMULTIGAUGE_BUILD_WEB=OFF
cmake --build build --config Debug --target multigauge_ci
ctest --test-dir build/core -C Debug --output-on-failure
```

Build firmware when shared code or ESP32 code is affected:

```powershell
pio run -d ports/esp32 -e esp32-s3-devkitc-1
```

Web builds require an active/discoverable emsdk. The component-specific scripts and publication checks are in `ports/web/AGENTS.md`. CI's full graph builds core, then firmware, then web; select all affected targets locally.

## Working rules and definition of done

- Keep changes narrowly scoped. Do not mix unrelated refactoring, formatting, generated output, or dependency upgrades into a focused task.
- Add a dependency only with a concrete need and justification; update the owning dependency manifest/bootstrap source, not generated copies.
- Follow existing naming and layout in the touched component. For public C++ declarations, follow `docs/style/doxygen.md` and state non-obvious ownership, lifetime, nullability, units, and structured return shapes.
- Add or update the closest relevant tests when behavior changes. When no automated coverage exists, record the targeted manual checks performed.
- A change is complete when affected targets build, relevant tests pass, schemas/examples/consumers agree with intentional contract changes, generated artifacts can be reproduced by their scripts, embedded implications were considered, and the diff contains no unrelated files.
- Report commands that were not run or require emsdk, credentials, network access, or physical hardware.

## Maintaining these instructions

Treat every `AGENTS.md` as maintained repository documentation. When a change makes applicable guidance inaccurate, make the smallest necessary update in the nearest owning `AGENTS.md`, preserve still-correct guidance, revise or remove obsolete instructions, and verify changed commands against current configuration. This includes durable changes to directory layout or ownership, build/test/lint/format/generation commands, platforms/toolchains/language standards, architectural constraints, public API or serialization compatibility, generated-file rules, environment setup, or the definition of done.

Do not update `AGENTS.md` for temporary implementation details or changes that do not affect future agent behavior. Avoid duplicating information already owned by another `AGENTS.md` or authoritative document; link to it instead.
