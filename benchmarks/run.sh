#!/usr/bin/env bash
#
# Convenience wrapper around `vendor/bin/phpbench run`.
#
# For a locally-built extension (the .so/.dylib lives under out/ rather than
# being installed in the PHP extension_dir) PhpBench needs to be told both which
# PHP to spawn and where the module is. This script autodetects a dev build and
# supplies the right --php-binary / --php-config flags; when the extension is
# installed system-wide it adds nothing and lets phpbench.json drive.
#
# Usage:
#   benchmarks/run.sh                      # everything, aggregate report
#   benchmarks/run.sh --group=offline      # offline micro-benchmarks only
#   benchmarks/run.sh --group=live         # live (needs a running ScyllaDB)
#   benchmarks/run.sh --store --tag=before # snapshot a baseline
#   benchmarks/run.sh --ref=before --report=aggregate --report=diff
#
# Overrides:
#   PHP=/path/to/php          PHP binary to run benchmarks with
#   EXT=/path/to/cassandra.so extension module to load
#   SCYLLADB_HOSTS, SCYLLADB_PORT, SCYLLADB_USERNAME, SCYLLADB_PASSWORD,
#   SCYLLADB_BENCH_KEYSPACE   live-benchmark connection settings

set -euo pipefail

cd "$(dirname "$0")/.."

PHP_BIN="${PHP:-php}"

# Locate a locally-built module unless one was passed explicitly.
if [[ -z "${EXT:-}" ]]; then
    EXT="$(find out -maxdepth 2 \( -name 'cassandra.so' -o -name 'cassandra.dylib' \) 2>/dev/null \
        | xargs -r ls -t 2>/dev/null | head -n1 || true)"
fi

ARGS=(--php-binary="$PHP_BIN")
if [[ -n "${EXT:-}" && -f "$EXT" ]]; then
    ARGS+=(--php-config="{\"extension\":\"$(cd "$(dirname "$EXT")" && pwd)/$(basename "$EXT")\"}")
    echo "→ using local extension: $EXT" >&2
else
    echo "→ no local build found under out/; relying on installed 'cassandra' extension" >&2
fi

# Default to the aggregate report when the caller passes no report/action flags.
if [[ $# -eq 0 ]]; then
    set -- --report=aggregate
fi

exec vendor/bin/phpbench run "${ARGS[@]}" "$@"
