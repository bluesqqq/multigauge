# Package storage

Multigauge stores runtime data below an implementation-selected data root:

```text
<dataRoot>/
  library.json
  state.json
  packages/<package-id>/
    manifest.json
    faces/<face-id>.json
```

`package::Manager::importPackage` accepts [package.schema.json](./schemas/package.schema.json), assigns
internal package and face IDs, and writes individual face documents. `exportPackage`
reconstructs the package document without those internal IDs.

`manifest.json`, `library.json`, and each face document follow
[manifest.schema.json](./schemas/manifest.schema.json),
[library.schema.json](./schemas/library.schema.json), and
[gauge.schema.json](./schemas/gauge.schema.json), respectively.

`state.json` follows [state.schema.json](./schemas/state.schema.json). It is
device-local configuration owned by `mg::sensor::Manager`: provider instances,
user-defined values, and sensor bindings. It is not included when importing or
exporting a gauge package and does not store live sensor readings.
