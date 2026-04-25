#!/usr/bin/env bash
set -euo pipefail

DRIVER="scylladb"
INSTALL_PREFIX="/usr/local"
BUILD_DIR=""
KEEP_SRC=0
BUILD_TYPE="RelWithDebInfo"
GIT_REF="HEAD"

print_usage() {
    cat <<EOF

Usage: $(basename "$0") [OPTIONS]

Options:
  --driver NAME     Driver to build: scylladb (default) or cassandra
  --prefix PATH     Install prefix (default: /usr/local)
  --ref GIT_REF     Git branch/tag/commit to check out (default: HEAD of default branch)
  --build-type TYPE CMake build type: Debug|Release|RelWithDebInfo (default: RelWithDebInfo)
  --build-dir PATH  Temporary build directory (default: auto in /tmp)
  --keep-src        Keep source directory after install
  -h, --help        Show this help

Examples:
  $(basename "$0")                                  # scylladb driver to /usr/local
  $(basename "$0") --driver cassandra --prefix \$HOME/.local
  $(basename "$0") --prefix /opt/scylladb --build-type Release

EOF
}

die() { echo "ERROR: $*" >&2; exit 1; }

while [[ $# -gt 0 ]]; do
    case "$1" in
        --driver)     DRIVER="${2:?--driver requires a value}"; shift 2 ;;
        --prefix)     INSTALL_PREFIX="${2:?--prefix requires a value}"; shift 2 ;;
        --ref)        GIT_REF="${2:?--ref requires a value}"; shift 2 ;;
        --build-type) BUILD_TYPE="${2:?--build-type requires a value}"; shift 2 ;;
        --build-dir)  BUILD_DIR="${2:?--build-dir requires a value}"; shift 2 ;;
        --keep-src)   KEEP_SRC=1; shift ;;
        -h|--help)    print_usage; exit 0 ;;
        *) die "Unknown option: $1. Use --help for usage." ;;
    esac
done

[[ "$DRIVER" == "scylladb" || "$DRIVER" == "cassandra" ]] \
    || die "Invalid --driver '$DRIVER'. Must be 'scylladb' or 'cassandra'."

command -v cmake >/dev/null 2>&1 || die "cmake not found in PATH"
command -v git   >/dev/null 2>&1 || die "git not found in PATH"
command -v ninja >/dev/null 2>&1 || die "ninja not found in PATH"

GIT_REPO="https://github.com/${DRIVER}/cpp-driver.git"

install_system_deps() {
    case "$(uname -s)" in
        Darwin)
            echo "==> Installing build deps (macOS)"
            command -v brew >/dev/null 2>&1 || die "Homebrew not found. Install from https://brew.sh"
            brew install cmake ninja pkg-config openssl
            ;;
        Linux)
            # shellcheck source=/dev/null
            . /etc/os-release 2>/dev/null || return
            case "${ID:-}" in
                fedora)
                    echo "==> Installing build deps (Fedora)"
                    dnf install -y cmake pkg-config gcc ninja-build openssl-devel openssl-devel-engine
                    ;;
                ubuntu|debian)
                    echo "==> Installing build deps (Ubuntu/Debian)"
                    apt-get install -y pkg-config build-essential libssl-dev
                    ;;
                *)
                    echo "WARN: Unknown distro '${ID:-}', skipping automatic dep install" >&2
                    ;;
            esac
            ;;
    esac
}

if [[ -z "$BUILD_DIR" ]]; then
    BUILD_DIR="$(mktemp -d /tmp/cpp-driver-build-XXXXXX)"
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

echo "==> Cloning $DRIVER/cpp-driver into $SRC_DIR"
git clone --depth 1 "$GIT_REPO" "$SRC_DIR"

if [[ "$GIT_REF" != "HEAD" ]]; then
    git -C "$SRC_DIR" fetch --depth 1 origin "$GIT_REF"
    git -C "$SRC_DIR" checkout FETCH_HEAD
fi

NPROC="$(nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)"

# timerfd is Linux-only — must be OFF on macOS
TIMERFD="ON"
[[ "$(uname -s)" == "Darwin" ]] && TIMERFD="OFF"

echo "==> Configuring (prefix=$INSTALL_PREFIX, build=$BUILD_TYPE)"
CFLAGS="-fPIC" \
CXXFLAGS="-fPIC -Wno-error=redundant-move" \
LDFLAGS="-flto" \
LIBUV_ROOT_DIR_DETECTED=""
if command -v brew >/dev/null 2>&1; then
    LIBUV_ROOT_DIR_DETECTED="$(brew --prefix libuv 2>/dev/null || true)"
fi

cmake -G Ninja -B "$SRC_DIR/build" -S "$SRC_DIR" \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    ${LIBUV_ROOT_DIR_DETECTED:+-DLIBUV_ROOT_DIR="$LIBUV_ROOT_DIR_DETECTED"} \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCASS_CPP_STANDARD=17 \
    -DCASS_BUILD_STATIC=ON \
    -DCASS_BUILD_SHARED=ON \
    -DCASS_USE_STD_ATOMIC=ON \
    -DCASS_USE_STATIC_LIBS=ON \
    -DCASS_USE_TIMERFD="$TIMERFD" \
    -DCASS_USE_LIBSSH2=ON \
    -DCASS_USE_ZLIB=ON \
    -DCMAKE_CXX_COMPILER_LAUNCHER="${CCACHE_DIR:+ccache}" \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=OFF \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

echo "==> Building (jobs=$NPROC)"
cmake --build "$SRC_DIR/build" --parallel "$NPROC"

echo "==> Installing to $INSTALL_PREFIX"
cmake --install "$SRC_DIR/build"

echo "==> $DRIVER cpp-driver installed to $INSTALL_PREFIX"
