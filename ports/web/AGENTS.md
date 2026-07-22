# WebAssembly Port Guide

This file refines the repository guide for `ports/web/`.

## Architecture and compatibility

- This is the Emscripten/browser port of the C++20 core, not the Flask application. `runtime/src/platform/` supplies browser graphics, filesystem, time, and logging; `runtime/bindings/` is the C ABI consumed by JavaScript; `runtime/js/pageRenderer.js` is the shared loader.
- CMake must run with the Emscripten toolchain. `CMakePresets.json` requires CMake 3.25+, Ninja, and an `EMSDK` whose toolchain is at `upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake`.
- Treat exported `mg_runtime_*`/`mg_editor_*` names, parameter conventions, handle sentinels, JSON result shapes, and JS loader paths as public compatibility surfaces. Keep implementations, `EditorApi.cmake`/`RuntimeAPI.cmake`, and JS consumers in sync.
- Browser-specific behavior stays here; portable renderer/editor behavior stays in `core/`. The Emscripten filesystem data root is `/work`, and the build enables filesystem support and memory growth.

## Build and generated output

From `ports/web/`, the supported helper builds are:

```powershell
.\build.ps1
.\build.ps1 -Configuration Release
```

On Linux/macOS:

```bash
./build.sh
./build.sh Release
```

The equivalent presets are `wasm-debug` and `wasm-release`. Helper scripts discover emsdk from `EMSDK` or nearby `emsdk` directories.

- CMake build state is under `ports/web/build/`; browser deliverables are generated into ignored `ports/web/dist/`. Do not hand-edit either.
- After a successful build, `dist/js/pageRenderer.js`, `dist/wasm/multigauge.js`, and `dist/wasm/multigauge.wasm` must exist; source maps may also be produced.
- Publish the verified bundle from the repository root with `./scripts/publish-web-assets.ps1` (or `./scripts/publish-web-assets.sh`). The destination `apps/website/static/multigauge-web/` is generated and ignored.
- No automated browser test/lint command is configured. For binding, rendering, resize, or editor changes, build the affected configuration and manually exercise the relevant page under `examples/` or the Flask editor; report missing browser verification.
