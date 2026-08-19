# Contributing

Thanks for your interest in the ScyllaDB PHP Driver! This document explains how
to report issues, set up a development environment, and get a pull request
merged.

The extension is a PHP 8.x C/C++ extension being **migrated from C++ to C23**.
Before writing code, please skim [`CLAUDE.md`](CLAUDE.md) — it is the canonical
guide to the module architecture, coding standards, and build system.

- **Branching & release model:** [`BRANCHING.md`](BRANCHING.md)
- **Code of conduct:** be respectful. Discussion happens on the
  [ScyllaDB Developers Discord](https://discord.gg/B6rutCXvgp).

---

## Reporting bugs

Good bug reports save everyone time. Please include:

- Driver version (or commit SHA) and the branch you are on.
- PHP version and build (`php -v`, plus NTS/ZTS and debug/release).
- ScyllaDB or Cassandra version, and which backend driver you built against
  (`scylladb` or `cassandra`).
- Dependency versions: libuv, the ScyllaDB C/C++ driver, compiler and version.
- A complete stack trace and any relevant logs.
- A minimal reproduction: the smallest script and cluster configuration that
  triggers the problem.

Security issues should **not** be filed as public issues — see
[Security](#security) below.

---

## Development setup

The default `scylla-cpp` backend builds the vendored driver in
`third-party/cpp-driver`, so you do not install a C/C++ driver to build it. You
still need libuv, OpenSSL and a matching PHP build:

```bash
# Install native dependencies (prefix is configurable — see scripts/)
./scripts/compile-libuv.sh        --prefix ~/.local
./scripts/compile-php.sh -v 8.4 -o ./php

# Configure + build the extension (presets are generated — see below)
cmake --preset DebugPHP8.4NTS
cmake --build out/DebugPHP8.4NTS
```

CMake presets are generated from the supported PHP-version / thread-safety /
backend matrix. Regenerate them after changing versions:

```bash
php generate-presets.php   # rewrites CMakePresets.json
```

Preset names follow `<BuildType>PHP<version><NTS|ZTS>[Cassandra|ScyllaRust]`,
e.g. `DebugPHP8.4NTS`, `ReleasePHP8.3ZTSCassandra`.

### The vendored C++ driver

`third-party/cpp-driver` holds the ScyllaDB cpp-driver source. Upstream no
longer maintains it, so the driver ships with the extension and is fixed here.
`third-party/cpp-driver/UPSTREAM` records the imported commit, and
`git log -- third-party/cpp-driver` lists every local change.

To link a driver installed on the system instead:

```bash
./scripts/compile-cpp-driver.sh --driver scylladb --prefix ~/.local
cmake --preset DebugPHP8.4NTS -DPHP_SCYLLADB_USE_SYSTEM_DRIVER=ON
```

The `cassandra` and `scylla-rust` backends always come from the system.

---

## Making changes

### Coding standards

All rules live in [`CLAUDE.md`](CLAUDE.md). The essentials:

- **New code is C23** (`.c` files). Do not add new `.cpp` files; port a `.cpp`
  file to `.c` when you touch it substantially.
- Follow the **canonical module pattern** in [`src/Cluster/`](src/Cluster) —
  stub → generated arginfo → implementation → handlers.
- Use the modern parameter API (`ZEND_PARSE_PARAMETERS_START/END`), never
  `zend_parse_parameters()`.
- Use `emalloc`/`efree` (request) or `pemalloc`/`pefree` (persistent) — never
  raw `malloc`/`free`. Release `zend_string*` with `zend_string_release()`.
- C/C++ is formatted with `clang-format` and linted with `clang-tidy`
  (see [`.clang-format`](.clang-format) / [`.clang-tidy`](.clang-tidy)).
  Format your changes before committing.

### PHP-visible API changes (stubs)

The PHP-facing API is defined in `*.stub.php` files, which generate
`*_arginfo.h`. **Never edit the generated header by hand.** After changing a
stub, regenerate and commit both files:

```bash
php php/8.4-debug-nts/src/build/gen_stub.php src/MyModule/MyClass.stub.php
```

### Tests

Tests use [Pest](https://pestphp.com/). A running ScyllaDB is required for the
integration suite:

```bash
./scripts/run-scylladb.sh                     # start ScyllaDB via docker compose
cmake --preset DebugPHP8.4NTS && cmake --build out/DebugPHP8.4NTS
composer install
php ./vendor/bin/pest
```

Please add or update tests for any behavioral change. Bug fixes should include a
regression test that fails without the fix.

---

## Pull requests

1. **Fork** the repository and create a topic branch off `trunk`
   (see [`BRANCHING.md`](BRANCHING.md)). Name it `feat/…`, `fix/…`,
   `refactor/…`, `test/…`, `build/…`, or `docs/…`.
2. Keep the change focused. Unrelated cleanups belong in separate PRs.
3. **Commit messages** follow [Conventional Commits](https://www.conventionalcommits.org/):
   `type(scope): summary`, e.g. `fix(session): release prepared statement on error`.
   Common types: `feat`, `fix`, `refactor`, `perf`, `build`, `ci`, `test`,
   `docs`, `chore`.
4. Make sure it builds and tests pass on at least one preset locally. CI runs the
   full matrix (PHP 8.3–8.5, NTS/ZTS, ScyllaDB + Cassandra backends) on every PR.
5. Open the PR against **`trunk`**. Fill in what changed and why, and link any
   related issue.
6. A maintainer reviews and merges. Please keep the branch up to date and
   respond to review comments.

### CI must be green

Every PR runs the test matrix and lint checks. All GitHub Actions are **pinned
to full commit SHAs** (org policy). If you add or bump an action, pin it to a
40-character SHA with the version in a trailing comment:

```yaml
- uses: actions/checkout@34e114876b0b11c390a56381ad16ebd13914f8d5 # v4
```

### Backporting a merged fix

Fixes that also apply to another release line are backported automatically. Add
one (or both) of these labels to the PR — before or after merge — and a bot
cherry-picks the merged commit and opens a backport PR:

| Label | Onto | Line |
|-------|------|------|
| `backport/1.x` | `v1.x` | released 1.x maintenance |
| `backport/2.x` | `v2.x` | next major (2.x), in development |

If the cherry-pick conflicts, the bot still opens the PR (with conflict markers)
and comments the manual steps. See [`.github/workflows/backport.yml`](.github/workflows/backport.yml)
and [`BRANCHING.md`](BRANCHING.md).

---

## Security

Please report vulnerabilities privately rather than opening a public issue.
Contact the maintainer at the address in [`composer.json`](composer.json) /
[`.github/FUNDING.yml`](.github/FUNDING.yml), or through the ScyllaDB Developers
Discord, and allow time for a fix before public disclosure.

---

## License

By contributing, you agree that your contributions are licensed under the
[Apache License 2.0](LICENSE), the same license as the project.
