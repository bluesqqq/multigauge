# Releasing

Multigauge uses a single repository-wide version stored in [`../VERSION`](../VERSION).
Component directories such as `core/`, `ports/`, and `apps/website/` do not have
independent release versions.

## Normal development

Pull requests merge into `main`. Every push to `main` runs the full CI workflow,
but `main` is not released automatically.

## Creating a release

Run the **Prepare Release** workflow manually from GitHub Actions.
This workflow must use the `MULTIGAUGE_RELEASE_TOKEN` secret so the pushed tag
can trigger the production release workflow.

Choose one of:

- `patch`
- `minor`
- `major`

Or provide an explicit SemVer version such as `0.2.0`. A leading `v` is accepted
for convenience.

The workflow:

1. checks out `main`;
2. calculates the next version or uses the explicit version;
3. updates `VERSION`;
4. commits the version bump to `main`;
5. creates an annotated tag named `vX.Y.Z`;
6. pushes the tag.

The pushed tag triggers the **Production Release** workflow.

## Release notes

The **Prepare Release** workflow accepts manual release notes. If notes are
provided, they are stored in the annotated tag and used as the GitHub Release
body.

If notes are left empty, the workflow generates a commit summary from the
previous `v*` release tag to the new release commit. Manual notes are preferred
for public releases because commit summaries are mechanically correct but not
curated.

## Production artifacts

The **Production Release** workflow validates that the tag matches `VERSION`,
then builds release artifacts and creates a single GitHub Release containing:

- `firmware.bin`
- `multigauge-web-dist.zip`
