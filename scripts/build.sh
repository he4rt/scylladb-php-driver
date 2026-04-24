#!/usr/bin/env bash
set -euo pipefail

DRIVER="${1:-scylladb}"
BUILD_DIR="cmake-build"

die() { echo "ERROR: $*" >&2; exit 1; }

command -v cmake      >/dev/null 2>&1 || die "cmake not found in PATH"
command -v php-config >/dev/null 2>&1 || die "php-config not found in PATH"

[[ "$DRIVER" == "scylladb" || "$DRIVER" == "cassandra" ]] \
    || die "Unknown driver '$DRIVER'. Must be 'scylladb' or 'cassandra'."

USE_LIBCASSANDRA="OFF"
[[ "$DRIVER" == "cassandra" ]] && USE_LIBCASSANDRA="ON"

rm -rf "$BUILD_DIR"

cmake -B "$BUILD_DIR" \
    -DCUSTOM_PHP_CONFIG="$(command -v php-config)" \
    -DCMAKE_BUILD_TYPE=Release \
    -DENABLE_LTO=ON \
    -DLINK_LIBUV_STATIC=ON \
    -DPHP_DRIVER_STATIC=ON \
    -DUSE_LIBCASSANDRA="$USE_LIBCASSANDRA"

cmake --build "$BUILD_DIR" --parallel "$(nproc)"
cmake --install "$BUILD_DIR" --component extension

cp "$(php-config --extension-dir)/cassandra.so" .
