# Multigauge Storage Layout

This document describes the on-device file layout used by `multigauge-core`.

The core assumes one configurable data root:

```text
<dataRoot>/
  library.json
  state.json
  packages/
    <package-id>/
      manifest.json
      faces/
        <face-id>.json
      assets/
        ...
```

## Data Root

`<dataRoot>` is the base directory chosen by the platform implementation.

Examples:

- ESP32: `/multigauge`
- Linux: a user data directory such as `~/.local/share/multigauge`

The core should resolve all internal paths relative to this root.

## `library.json`

`library.json` is the installed-package index.

It exists to support fast browsing of installed content. It should be treated as derived data that can be rebuilt from the package manifests if needed.

The schema lives in [`docs/schemas/library.schema.json`](./schemas/library.schema.json).

Each package entry contains:

- `id`
- `name`
- `author`
- `description`
- `favorite`

`id` is the internal package id. `name` is the user-facing name shown in the library.

## `state.json`

`state.json` stores persisted user/runtime state.

## `packages/`

`packages/` contains all installed gauge packages.

Each package has its own directory keyed by package id.

### `manifest.json`

`manifest.json` is the canonical package description.

The schema lives in [`docs/schemas/manifest.schema.json`](./schemas/manifest.schema.json).

Current manifest fields:

- `id`
- `name`
- `author`
- `description`
- `faces`

Each face entry contains:

- `id`
- `name`
- `path`

`id` is the internal face id. It is generated from the user-facing face `name` by lowercasing and replacing spaces or punctuation with hyphens. If an id already exists within the package, a numeric suffix is added.

### `package.schema.json`

`package.schema.json` is the canonical schema for the package document used by the editor, `importPackage`, and `exportPackage`.

The schema lives in [`docs/schemas/package.schema.json`](./schemas/package.schema.json).

Current package fields:

- `id`
- `name`
- `author`
- `description`
- `faces`
- `assets` in packages that embed external files

Each face entry contains:

- `id`
- `name`
- `path`
- `face`

`face` is the opaque face payload loaded by `GaugeFace`.

### `faces/`

`faces/` contains the gauge face JSON files for the package.

Each face is stored as a separate JSON file.

If the uploaded document contains 3 faces, the importer writes 3 separate files into `faces/`, one per face.

The face file name uses the generated face id, for example `faces/main-cluster.json`.

### `assets/`

`assets/` contains imported package assets such as:

- images
- fonts
- other face resources

Assets are stored once on disk and loaded at runtime when needed.

Assets are package-local only. A package should not reference assets from another package.

## Import and Export Behavior

When a package is imported:

1. The package content is written under `packages/<package-id>/`
2. Each uploaded face is saved as its own `faces/<face-id>.json` file
3. Embedded assets are extracted into `assets/`
4. Face JSON is stripped so it references the extracted asset files
5. `library.json` is updated

When a package is exported:

1. The package is exported using the canonical package schema
2. The exported package contains the same one-file-per-face layout under `faces/`
3. Asset bytes may be re-embedded when available

## Source of Truth

The source of truth is:

- `packages/<package-id>/manifest.json`
- `packages/<package-id>/faces/<face-id>.json`
- `packages/<package-id>/assets/`
- `state.json` for user/runtime state

`library.json` is an index and may be regenerated.
