#!/usr/bin/env bash
# Generate committed benchmark baselines under benchmarks/baselines/.
#
# Runs the offline group (deterministic, no DB) always and the live group
# best-effort (needs a reachable ScyllaDB node — skipped if it fails), dumps
# each run to a PhpBench XML, and parses it into a portable JSON + Markdown
# baseline via benchmarks/baseline.php.
#
# Honours the same env as benchmarks/run.sh (PHP, EXT, SCYLLADB_*).
#
# Usage: benchmarks/baseline.sh [--offline-only]
set -euo pipefail

HERE="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT="$(cd "$HERE/.." && pwd)"
cd "$ROOT"

PHP_BIN="${PHP:-php}"
TMPDIR_BL="$(mktemp -d)"
trap 'rm -rf "$TMPDIR_BL"' EXIT

run_group() {
    local group="$1"
    local dump="$TMPDIR_BL/$group.xml"

    echo "→ benchmarking group '$group'…" >&2
    if bash "$HERE/run.sh" --group="$group" --progress=none --dump-file="$dump"; then
        "$PHP_BIN" "$HERE/baseline.php" parse "$group" "$dump"
    else
        echo "→ group '$group' failed (node unreachable?) — skipping baseline" >&2
        return 1
    fi
}

run_group offline || true

if [[ "${1:-}" != "--offline-only" ]]; then
    run_group live || true
fi

"$PHP_BIN" "$HERE/baseline.php" readme
echo "→ baselines written to benchmarks/baselines/" >&2
