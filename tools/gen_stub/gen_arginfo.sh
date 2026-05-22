#!/usr/bin/env bash
# tools/gen_stub/gen_arginfo.sh
#
# Wrapper around PHP's official build/gen_stub.php.
#
# Usage: gen_arginfo.sh <stub-file> <php-binary> <gen-stub-script>
#
# The wrapper adds two minimal fixups around the upstream tool so we don't
# have to vendor a patched copy:
#
#  1. Pre-process: gen_stub upstream errors on `declare(strict_types=1);`
#     ("Unexpected node Stmt_Declare"). The declare is harmless for arginfo
#     output, so we strip it from a tmp copy and feed that to gen_stub.
#
#  2. Post-process: emitted calls to `zend_add_function_attribute(...)` /
#     `zend_add_parameter_attribute(...)` pass `zend_hash_str_find_ptr(...)`
#     (returns void*) where a `zend_function *` is expected. C accepts the
#     implicit conversion; C++ doesn't, and our extension is compiled as
#     C++. We inject an explicit cast.
#
# Both fixups are candidates for upstreaming to PHP; until then this
# wrapper keeps the maintenance surface small (a handful of sed lines vs
# a vendored 6500-line gen_stub.php fork).

set -eu

stub="$1"
php_bin="$2"
gen_stub="$3"

if [ ! -f "$stub" ]; then
    echo "gen_arginfo.sh: stub not found: $stub" >&2
    exit 1
fi
if [ ! -x "$php_bin" ] && ! command -v "$php_bin" > /dev/null; then
    echo "gen_arginfo.sh: php binary not executable: $php_bin" >&2
    exit 1
fi
if [ ! -f "$gen_stub" ]; then
    echo "gen_arginfo.sh: gen_stub.php not found: $gen_stub" >&2
    exit 1
fi

stub_dir=$(dirname "$stub")
stub_base=$(basename "$stub" .stub.php)
arginfo="$stub_dir/${stub_base}_arginfo.h"

# Pre-process: strip `declare(strict_types=1);` (harmless for arginfo).
# gen_stub looks for files named *.stub.php, so the tmp file must use that
# suffix exactly. Use a per-invocation tmp directory to avoid name clashes.
tmpdir=$(mktemp -d)
trap 'rm -rf "$tmpdir"' EXIT

tmpstub="$tmpdir/${stub_base}.stub.php"

# Match `declare(strict_types=1);` exactly with optional surrounding whitespace.
sed -E 's/^[[:space:]]*declare[[:space:]]*\([[:space:]]*strict_types[[:space:]]*=[[:space:]]*1[[:space:]]*\)[[:space:]]*;[[:space:]]*$//' \
    "$stub" > "$tmpstub"

# Run upstream gen_stub. It writes alongside the input, so the output
# lands at $tmpdir/${stub_base}_arginfo.h.
"$php_bin" "$gen_stub" "$tmpstub" > /dev/null

# Move the generated header next to the real stub.
mv "$tmpdir/${stub_base}_arginfo.h" "$arginfo"

# Post-process: cast void* → zend_function* for C++ compatibility.
# Use a portable sed (no -i / -E differences between GNU and BSD).
tmp_out=$(mktemp -t "${stub_base}_arginfo.XXXXXX").h
sed \
    -e 's/zend_add_function_attribute(zend_hash_str_find_ptr/zend_add_function_attribute((zend_function *)zend_hash_str_find_ptr/g' \
    -e 's/zend_add_parameter_attribute(zend_hash_str_find_ptr/zend_add_parameter_attribute((zend_function *)zend_hash_str_find_ptr/g' \
    "$arginfo" > "$tmp_out"
mv "$tmp_out" "$arginfo"
