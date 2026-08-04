#!/bin/sh
# Verify that cassandra.so can resolve every symbol it references using only
# the libraries it declares itself (DT_NEEDED) plus the PHP binary that will
# host it.
#
# This is the regression guard for gh-117, where the module referenced libgmp's
# mpz_* symbols without listing libgmp as a dependency. Nothing failed at build
# time (ELF shared objects may carry unresolved symbols), and nothing failed at
# load time on the maintainers' machines either, for two compounding reasons:
#
#   * PHP dlopen()s extensions with RTLD_LAZY|RTLD_GLOBAL. glibc binds lazily,
#     so a missing dependency stays invisible until the symbol is first called
#     — and every earlier-loaded extension donates its symbols to us, so an
#     ext/gmp that happened to load first silently supplied them. musl has no
#     lazy binding, so on Alpine the same file died at dlopen with
#     "Error relocating ...: __gmpz_cmp_ui: symbol not found".
#   * On some builds the php binary itself links libgmp, which masks the bug
#     even under LD_BIND_NOW. Loading the module proves nothing on such a host.
#
# Hence this static check: it never consults the ambient process, only what the
# module declares and what PHP itself exports.
#
# Usage: check-module-symbols.sh <module.so> <php-binary> [--warn-only]
#
# --warn-only reports the same findings but exits 0. Used for the experimental
# scylla-rust backend, which is knowingly incomplete: cpp-rs-driver does not
# implement the cass_*_meta_* schema APIs the extension calls, so that build
# has unresolved symbols regardless of how it is linked. Failing on it would
# leave a permanently red job and say nothing new.

set -eu

MODULE=${1:?usage: check-module-symbols.sh <module.so> <php-binary> [--warn-only]}
PHP_BIN=${2:?usage: check-module-symbols.sh <module.so> <php-binary> [--warn-only]}
MODE=${3:-}

if [ "$(uname -s)" = "Darwin" ]; then
    echo "check-module-symbols: skipped (macOS links extensions with -undefined dynamic_lookup)"
    exit 0
fi

for tool in nm ldd; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "check-module-symbols: skipped ($tool not found; install binutils to enable)"
        exit 0
    fi
done

work=$(mktemp -d)
# shellcheck disable=SC2064
trap "rm -rf '$work'" EXIT

# Undefined symbols the module needs. Type 'U' only: 'w'/'v' are weak
# undefined, which are legitimately allowed to stay unresolved.
#
# Version suffixes are stripped throughout: a reference reads "strncmp@GLIBC_2.17"
# while the definition reads "strncmp@@GLIBC_2.17" (double @ marks the default
# version), so the two only compare equal once the suffix is gone. We are asking
# "does any declared library provide this name at all", not resolving versions.
nm -D --undefined-only -f posix "$MODULE" \
    | awk '$2 == "U" { sub(/@.*/, "", $1); print $1 }' \
    | sort -u > "$work/undefined"

# Everything resolvable at load time: the module's own declared dependency
# chain, plus the PHP binary (which exports the Zend/PHP API to extensions).
# Deliberately NOT the host's other extensions or PHP's unrelated dependencies —
# relying on those is precisely the bug this guards against.
{
    ldd "$MODULE" 2>/dev/null | awk '/=> \// { print $3 } /^\t\// { print $1 }'
    printf '%s\n' "$PHP_BIN"
} | sort -u > "$work/providers"

: > "$work/defined"
while IFS= read -r lib; do
    [ -e "$lib" ] || continue
    nm -D --defined-only -f posix "$lib" 2>/dev/null \
        | awk '{ sub(/@.*/, "", $1); print $1 }' >> "$work/defined"
done < "$work/providers"
sort -u -o "$work/defined" "$work/defined"

comm -23 "$work/undefined" "$work/defined" > "$work/missing"

if [ -s "$work/missing" ]; then
    if [ "$MODE" = "--warn-only" ]; then
        _verdict="WARN"
    else
        _verdict="FAIL"
    fi
    echo "check-module-symbols: ${_verdict} — $(basename "$MODULE") references symbols that"
    echo "nothing it depends on provides. It will fail to load on musl (Alpine), and"
    echo "will misbehave or crash elsewhere once these are called."
    echo
    echo "Declared dependencies (DT_NEEDED):"
    sed 's/^/    /' "$work/providers"
    echo
    echo "Unresolvable symbols:"
    sed 's/^/    /' "$work/missing"
    echo
    echo "Fix: link the providing library into the target (see scylladb_php_library()"
    echo "in cmake/TargetOptimizations.cmake) rather than relying on another PHP"
    echo "extension to have been loaded first."

    if [ "$_verdict" = "WARN" ]; then
        echo
        echo "Not failing the build: this backend is known to be incomplete."
        exit 0
    fi
    exit 1
fi

echo "check-module-symbols: OK — all $(wc -l < "$work/undefined" | tr -d ' ') undefined symbols resolve via declared dependencies"
