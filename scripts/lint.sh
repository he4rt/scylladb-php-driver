#!/usr/bin/env bash
# Run clang-tidy against the extension sources, mirroring what CI does.
#
# Usage:
#   scripts/lint.sh                # lint everything under src/
#   scripts/lint.sh --changed      # lint only files changed vs origin/v1.3.x
#   scripts/lint.sh <file> [...]   # lint specific files
#
# Env overrides:
#   PRESET        CMake preset name           (default: DebugPHP8.5NTS)
#   BUILD_DIR     Build directory              (default: out/$PRESET)
#   CLANG_TIDY    Path to clang-tidy binary    (default: auto-detect)
#   JOBS          Parallel jobs                (default: nproc / sysctl)
#   BASE_REF      Ref for --changed comparison (default: origin/v1.3.x)

set -euo pipefail

PRESET="${PRESET:-DebugPHP8.5NTS}"
BUILD_DIR="${BUILD_DIR:-out/${PRESET}}"
BASE_REF="${BASE_REF:-origin/v1.3.x}"

# Detect clang-tidy: prefer Homebrew LLVM on macOS (Apple Clang doesn't ship it).
if [[ -z "${CLANG_TIDY:-}" ]]; then
    for candidate in \
        /opt/homebrew/opt/llvm/bin/clang-tidy \
        /usr/local/opt/llvm/bin/clang-tidy \
        clang-tidy-19 \
        clang-tidy; do
        if command -v "$candidate" >/dev/null 2>&1; then
            CLANG_TIDY="$candidate"
            break
        fi
    done
fi
if [[ -z "${CLANG_TIDY:-}" ]] || ! command -v "$CLANG_TIDY" >/dev/null 2>&1; then
    echo "error: clang-tidy not found. Install via 'brew install llvm' or apt 'clang-tidy-19'." >&2
    exit 127
fi

# Parallelism
if [[ -z "${JOBS:-}" ]]; then
    if command -v nproc >/dev/null 2>&1; then
        JOBS=$(nproc)
    else
        JOBS=$(sysctl -n hw.logicalcpu 2>/dev/null || echo 4)
    fi
fi

# Extra args. On macOS, Homebrew LLVM needs the Xcode SDK pointed out.
EXTRA_ARGS=()
if [[ "$(uname -s)" == "Darwin" ]] && command -v xcrun >/dev/null 2>&1; then
    SDK=$(xcrun --show-sdk-path)
    EXTRA_ARGS+=("--extra-arg=-isysroot${SDK}")
fi

# Configure if needed
if [[ ! -f "${BUILD_DIR}/compile_commands.json" ]]; then
    echo "==> Configuring ${PRESET} (compile_commands.json not found)"
    cmake --preset "${PRESET}" \
        -DCUSTOM_PHP_CONFIG="$(command -v php-config)" \
        -DENABLE_SANITIZERS=OFF \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

# Decide what to lint
files=()
if [[ "${1:-}" == "--changed" ]]; then
    while IFS= read -r f; do
        [[ -n "$f" ]] && files+=("$f")
    done < <(git diff --name-only --diff-filter=AM "${BASE_REF}"...HEAD -- 'src/*.c' 'src/*.cc' 'src/*.cpp')
    if [[ ${#files[@]} -eq 0 ]]; then
        echo "No changed C/C++ files vs ${BASE_REF}."
        exit 0
    fi
elif [[ $# -gt 0 ]]; then
    files=("$@")
else
    while IFS= read -r f; do
        files+=("$f")
    done < <(find src -type f \( -name '*.c' -o -name '*.cc' -o -name '*.cpp' \))
fi

echo "==> clang-tidy: ${CLANG_TIDY} ($(${CLANG_TIDY} --version | head -1))"
echo "==> ${#files[@]} file(s), ${JOBS} parallel"

printf '%s\n' "${files[@]}" \
    | xargs -n1 -P"${JOBS}" "${CLANG_TIDY}" -p "${BUILD_DIR}" --quiet "${EXTRA_ARGS[@]}"
