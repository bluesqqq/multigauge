# Package storage

The public `PackageManager` API stores data below `AppConfig::dataRoot`:

```text
<dataRoot>/
  library.json
  state.json
  packages/<package-id>/
    manifest.json
    faces/<face-id>.json
```

`importPackage` accepts [package.schema.json](./schemas/package.schema.json), assigns
internal package and face IDs, and writes individual face documents. `exportPackage`
reconstructs the package document without those internal IDs.

`manifest.json`, `library.json`, and each face document follow
[manifest.schema.json](./schemas/manifest.schema.json),
[library.schema.json](./schemas/library.schema.json), and
[gauge.schema.json](./schemas/gauge.schema.json), respectively.

`state.json` follows [state.schema.json](./schemas/state.schema.json). It is
device-local runtime configuration for telemetry providers, user-defined values,
sensor bindings, and derived values. It is not included when importing or
exporting a gauge package and does not store live sensor readings.
