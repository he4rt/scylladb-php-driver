#!/usr/bin/env bash
# Build & install scylladb/cpp-rs-driver (the Rust-backed cpp-driver rewrite).
# Companion to scripts/compile-cpp-driver.sh — installs to a custom prefix so
# multiple driver backends can coexist on the same machine.
set -euo pipefail

INSTALL_PREFIX="/usr/local"
BUILD_DIR=""
KEEP_SRC=0
BUILD_TYPE="RelWithDebInfo"
GIT_REF="master"
GIT_REPO="https://github.com/scylladb/cpp-rs-driver.git"

print_usage() {
    cat <<EOF

Usage: $(basename "$0") [OPTIONS]

Builds scylladb/cpp-rs-driver — the API-compatible Rust-backed rewrite of
the C/C++ driver. Requires cargo (Rust toolchain) in PATH.

Options:
  --prefix PATH     Install prefix (default: /usr/local)
  --ref GIT_REF     Git branch/tag/commit (default: ${GIT_REF})
  --build-type TYPE Debug | Release | RelWithDebInfo (default: ${BUILD_TYPE})
  --build-dir PATH  Temporary build directory (default: auto in /tmp)
  --keep-src        Keep source directory after install
  -h, --help        Show this help

Examples:
  $(basename "$0") --prefix \$HOME/.local/scylla-rust
  $(basename "$0") --ref master --build-type Release

After install, point CMake at this prefix with PKG_CONFIG_PATH:

  PKG_CONFIG_PATH=\$HOME/.local/scylla-rust/lib/pkgconfig:\$PKG_CONFIG_PATH \\
    cmake --preset DebugPHP8.4NTSScyllaRust

EOF
}

die() { echo "ERROR: $*" >&2; exit 1; }

# sudo-if-needed wrapper for Linux package managers / installs.
SUDO=""
maybe_sudo() {
    if [[ $EUID -ne 0 ]] && command -v sudo >/dev/null 2>&1; then
        SUDO="sudo"
    fi
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --prefix)     INSTALL_PREFIX="${2:?--prefix requires a value}"; shift 2 ;;
        --ref)        GIT_REF="${2:?--ref requires a value}"; shift 2 ;;
        --build-type) BUILD_TYPE="${2:?--build-type requires a value}"; shift 2 ;;
        --build-dir)  BUILD_DIR="${2:?--build-dir requires a value}"; shift 2 ;;
        --keep-src)   KEEP_SRC=1; shift ;;
        -h|--help)    print_usage; exit 0 ;;
        *) die "Unknown option: $1. Use --help for usage." ;;
    esac
done

command -v cmake >/dev/null 2>&1 || die "cmake not found in PATH"
command -v git   >/dev/null 2>&1 || die "git not found in PATH"
command -v cargo >/dev/null 2>&1 || die "cargo not found in PATH (install Rust via https://rustup.rs)"

install_system_deps() {
    case "$(uname -s)" in
        Darwin)
            echo "==> Installing build deps (macOS)"
            command -v brew >/dev/null 2>&1 || die "Homebrew not found. Install from https://brew.sh"
            brew install cmake pkg-config openssl
            ;;
        Linux)
            maybe_sudo
            # shellcheck source=/dev/null
            . /etc/os-release 2>/dev/null || return
            case "${ID:-}" in
                fedora)
                    echo "==> Installing build deps (Fedora)"
                    $SUDO dnf install -y cmake pkg-config gcc openssl-devel openssl-devel-engine
                    ;;
                ubuntu|debian)
                    echo "==> Installing build deps (Ubuntu/Debian)"
                    $SUDO apt-get update
                    $SUDO apt-get install -y pkg-config build-essential libssl-dev cmake
                    ;;
                *)
                    echo "WARN: Unknown distro '${ID:-}', skipping automatic dep install" >&2
                    ;;
            esac
            ;;
    esac
}

if [[ -z "$BUILD_DIR" ]]; then
    BUILD_DIR="$(mktemp -d /tmp/cpp-rs-driver-build-XXXXXX)"
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

echo "==> Cloning scylladb/cpp-rs-driver (${GIT_REF}) into $SRC_DIR"
git clone --depth 1 --branch "$GIT_REF" "$GIT_REPO" "$SRC_DIR" 2>/dev/null \
    || { git clone "$GIT_REPO" "$SRC_DIR"; git -C "$SRC_DIR" checkout "$GIT_REF"; }

NPROC="$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)"

# OpenSSL hint for cargo's openssl-sys on macOS
if [[ "$(uname -s)" == "Darwin" ]] && command -v brew >/dev/null 2>&1; then
    if _ossl="$(brew --prefix openssl@3 2>/dev/null)" && [[ -d "$_ossl" ]]; then
        export OPENSSL_DIR="$_ossl"
        echo "==> OPENSSL_DIR=$OPENSSL_DIR"
    fi
fi

# cmake 4.x marks Rust language support as experimental and requires opt-in
# via a per-version UUID. The cpp-rs-driver's CMakeLists calls
# enable_language(Rust), so we set the variable that matches the active cmake.
# UUIDs:
#   3.31+ (stable Rust experimental): e3739111-7af2-4fdb-9d27-d4a31a4d2eda
#   4.3.x:                            3cc9b32c-47d3-4056-8953-d74e69fc0d6c
# We pass both — cmake only validates the value for its own version.
CMAKE_RUST_OPTS=(
    -DCMAKE_EXPERIMENTAL_RUST=3cc9b32c-47d3-4056-8953-d74e69fc0d6c
)

echo "==> Configuring (prefix=$INSTALL_PREFIX, build=$BUILD_TYPE)"
cmake -B "$SRC_DIR/build" -S "$SRC_DIR" \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCASS_BUILD_SHARED=ON \
    -DCASS_BUILD_STATIC=ON \
    "${CMAKE_RUST_OPTS[@]}" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

echo "==> Building (jobs=$NPROC)"
cmake --build "$SRC_DIR/build" --parallel "$NPROC"

# Use sudo for install if writing under /usr or /opt and we're not root.
INSTALL_SUDO=""
if [[ "$(uname -s)" != "Darwin" && $EUID -ne 0 ]]; then
    case "$INSTALL_PREFIX" in
        /usr/*|/opt/*) command -v sudo >/dev/null 2>&1 && INSTALL_SUDO="sudo" ;;
    esac
fi

echo "==> Installing to $INSTALL_PREFIX"
$INSTALL_SUDO cmake --install "$SRC_DIR/build"

echo "==> scylladb/cpp-rs-driver (${GIT_REF}) installed to $INSTALL_PREFIX"
echo
echo "Next: export PKG_CONFIG_PATH=$INSTALL_PREFIX/lib/pkgconfig:\$PKG_CONFIG_PATH"
echo "      cmake -DPHP_DRIVER_BACKEND=scylla-rust ..."
