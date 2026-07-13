# Branching & Release Model

This document describes how branches, releases, and backports work in the
ScyllaDB PHP Driver. For the full contribution workflow see
[`CONTRIBUTING.md`](CONTRIBUTING.md).

## Long-lived branches

| Branch | Role |
|--------|------|
| **`trunk`** | Default branch. All new work lands here first. This is the bleeding edge. |
| **`v1.x`** | Maintenance line for the current, released 1.x series. Receives backported bug fixes and security patches. |
| **`v2.x`** | The **next major version** (2.x), under active development. Not yet released; receives fixes ported over from `trunk`. |
| **`gh-pages`** | Generated documentation site (do not hand-edit; published by tooling). |

Short-lived branches are prefixed by intent and are deleted after merge:

- `feat/…` — new features
- `fix/…` — bug fixes
- `refactor/…` — internal restructuring (e.g. the C++ → C23 migration)
- `perf/…` — performance work
- `build/…`, `ci/…` — build system and CI
- `test/…` — tests only
- `docs/…` — documentation

## Where work goes

```
                 topic branch (feat/…, fix/…)
                        │  PR + review
                        ▼
   ┌──────────────────────────────────────────┐
   │                 trunk                      │  ← default; all PRs target this
   └──────────────────────────────────────────┘
              │ backport/1.x        │ backport/2.x
              ▼                     ▼
        ┌──────────┐          ┌───────────────┐
        │  v1.x    │          │     v2.x      │
        │ 1.x      │          │  next major   │
        │ maint.   │          │ (in progress) │
        └──────────┘          └───────────────┘
              │                     │
           tag v1.*             tag v2.* (once released)
```

- **Open every PR against `trunk`.** Do not target `v1.x` / `v2.x` directly;
  fixes reach them via the backport bot so those lines stay in sync with `trunk`.
- A change that only makes sense for one line (e.g. a fix for an API that no
  longer exists on `trunk`, or work specific to the 2.x rewrite) is the rare
  exception — call it out in the PR and a maintainer will advise.

## Backporting

Backports are automated by [`.github/workflows/backport.yml`](.github/workflows/backport.yml)
(the `korthout/backport-action`, SHA-pinned). Add one or both labels to a PR —
before or after it is merged:

| Label | Cherry-picks the merged commit onto | Purpose |
|-------|-------------------------------------|---------|
| `backport/1.x` | `v1.x` | Backport a fix to the released 1.x maintenance line |
| `backport/2.x` | `v2.x` | Port a fix forward into the in-progress next major (2.x) |

On merge the bot cherry-picks the commit onto the target branch and opens a
`[Backport <branch>] …` pull request. If the cherry-pick applies cleanly the PR
is ready for review; if it conflicts, the PR is still opened with conflict
markers and the bot comments the manual `git` steps.

> **Note:** the label suffix (`1.x`) is intentionally *not* the branch name
> (`v1.x`); the workflow maps each label to its branch explicitly. Adding a new
> target branch means adding a new entry to that workflow's matrix — the label
> alone is not enough.

### Manual backport (fallback)

If you need to backport without the bot:

```bash
git switch v1.x
git switch -c backport/1.x-<short-desc>
git cherry-pick -x <merged-commit-sha>   # -x records the source commit
# resolve conflicts if any, then:
git push origin HEAD
# open a PR against v1.x
```

## Releases

Releases are tagged and published by
[`.github/workflows/release.yml`](.github/workflows/release.yml), which triggers
on `v*` tags. Tagging a release branch (`v1.x` for 1.x releases, `v2.x` once the
next major ships) builds the extension across the supported matrix
(PHP 8.3–8.5, NTS/ZTS, ScyllaDB + Cassandra), attaches the artifacts to a GitHub
Release, and regenerates the changelog.

- Version tags use the `v` prefix: `v1.3.0`, `v2.0.0-rc1`, etc.
- Pre-release tags containing `-rc`, `-pre`, `-alpha`, or `-beta` are published
  as GitHub pre-releases automatically.
- Versioning follows [Semantic Versioning](https://semver.org/): breaking
  changes bump the major, backward-compatible features the minor, fixes the
  patch.
