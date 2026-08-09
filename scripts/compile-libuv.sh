#!/usr/bin/env bash
set -euo pipefail

LIBUV_VERSION="v1.52.1"
LIBUV_REPO="https://github.com/libuv/libuv.git"
INSTALL_PREFIX="/usr/local"
BUILD_DIR=""
KEEP_SRC=0

print_usage() {
    cat <<EOF

Usage: $(basename "$0") [OPTIONS]

Options:
  --prefix PATH     Install prefix (default: /usr/local)
  --version TAG     libuv git tag (default: $LIBUV_VERSION)
  --build-dir PATH  Temporary build directory (default: auto in /tmp)
  --keep-src        Keep source directory after install
  -h, --help        Show this help

Examples:
  $(basename "$0")                          # install to /usr/local (may need sudo)
  $(basename "$0") --prefix \$HOME/.local   # install locally, no sudo needed
  $(basename "$0") --prefix /opt/libuv --version v1.48.0

EOF
}

die() { echo "ERROR: $*" >&2; exit 1; }

while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix)   INSTALL_PREFIX="${2:?--prefix requires a value}"; shift 2 ;;
        --version)  LIBUV_VERSION="${2:?--version requires a value}"; shift 2 ;;
        --build-dir) BUILD_DIR="${2:?--build-dir requires a value}"; shift 2 ;;
        --keep-src) KEEP_SRC=1; shift ;;
        -h|--help)  print_usage; exit 0 ;;
        *) die "Unknown option: $1. Use --help for usage." ;;
    esac
done

install_system_deps() {
    case "$(uname -s)" in
        Darwin)
            echo "==> Installing build deps (macOS)"
            command -v brew >/dev/null 2>&1 || die "Homebrew not found. Install from https://brew.sh"
            brew install cmake ninja git
            ;;
        Linux)
            local SUDO=""
            if [[ $EUID -ne 0 ]] && command -v sudo >/dev/null 2>&1; then
                SUDO="sudo"
            fi
            # shellcheck source=/dev/null
            . /etc/os-release 2>/dev/null || return
            case "${ID:-}" in
                fedora)
                    echo "==> Installing build deps (Fedora)"
                    $SUDO dnf install -y cmake git ninja-build
                    ;;
                ubuntu|debian)
                    echo "==> Installing build deps (Ubuntu/Debian)"
                    $SUDO apt-get update
                    $SUDO apt-get install -y cmake git ninja-build
                    ;;
                *)
                    echo "WARN: Unknown distro '${ID:-}', skipping automatic dep install" >&2
                    ;;
            esac
            ;;
    esac
}

command -v cmake  >/dev/null 2>&1 || die "cmake not found in PATH"
command -v git    >/dev/null 2>&1 || die "git not found in PATH"
command -v ninja  >/dev/null 2>&1 || die "ninja not found in PATH"

if [[ -z "$BUILD_DIR" ]]; then
    BUILD_DIR="$(mktemp -d /tmp/libuv-build-XXXXXX)"
    CLEAN_BUILD_DIR=1
else
    mkdir -p "$BUILD_DIR"
    CLEAN_BUILD_DIR=0
fi

cleanup() {
    if [[ "${CLEAN_BUILD_DIR:-0}" -eq 1 && -d "$BUILD_DIR" && "$KEEP_SRC" -eq 0 ]]; then
        rm -rf "$BUILD_DIR"
    fi
}
trap cleanup EXIT

SRC_DIR="$BUILD_DIR/src"

install_system_deps

echo "==> Cloning libuv $LIBUV_VERSION into $SRC_DIR"
git clone --depth=1 --branch "$LIBUV_VERSION" "$LIBUV_REPO" "$SRC_DIR"

echo "==> Configuring (prefix=$INSTALL_PREFIX)"
LDFLAGS="-flto" CFLAGS="-fPIC" cmake -G Ninja -B "$SRC_DIR/build" -S "$SRC_DIR" \
    -DBUILD_TESTING=OFF \
    -DBUILD_BENCHMARKS=OFF \
    -DLIBUV_BUILD_SHARED=OFF \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_BUILD_TYPE="Release"

NPROC="$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)"

echo "==> Building (jobs=$NPROC)"
cmake --build "$SRC_DIR/build" --parallel "$NPROC"

echo "==> Installing to $INSTALL_PREFIX"
cmake --install "$SRC_DIR/build"

echo "==> libuv $LIBUV_VERSION installed to $INSTALL_PREFIX"
