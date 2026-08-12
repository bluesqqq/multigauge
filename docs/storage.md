# Package storage

`PackageManager` stores package data below the host-selected `AppConfig::dataRoot`:

```text
<dataRoot>/
  library.json
  state.json
  packages/
    <package-id>/
      manifest.json
      faces/<face-id>.json
```

`PackageManager::importPackage` accepts the user-facing package document described by
[package.schema.json](./schemas/package.schema.json). It creates internal package and
face IDs, stores each face separately, and rebuilds the derived `library.json` index.

`manifest.json` records package metadata and internal face IDs; its format is
[manifest.schema.json](./schemas/manifest.schema.json). `library.json` is the derived
package index described by [library.schema.json](./schemas/library.schema.json).

Face documents are loaded by `GaugeFace` and follow
[gauge.schema.json](./schemas/gauge.schema.json).
