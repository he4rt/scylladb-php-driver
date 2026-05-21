#!/usr/bin/env bash
set -euo pipefail

DRIVER="scylladb"
INSTALL_PREFIX=""
LAYOUT="per-driver"   # per-driver | flat
BUILD_DIR=""
KEEP_SRC=0
BUILD_TYPE="RelWithDebInfo"
GIT_REF="HEAD"

print_usage() {
    cat <<EOF

Usage: $(basename "$0") [OPTIONS]

Options:
  --driver NAME       Driver to build: scylladb (default) or cassandra
  --prefix PATH       Install prefix root (default: /usr/local)
                      With --layout per-driver, the driver installs to <prefix>/<driver>
                      so scylladb and cassandra can coexist.
  --layout LAYOUT     per-driver (default) | flat
                      per-driver: install to <prefix>/<driver>
                      flat:       install directly to <prefix> (old behavior; may overwrite)
  --ref GIT_REF       Git branch/tag/commit to check out (default: HEAD of default branch)
  --build-type TYPE   CMake build type: Debug|Release|RelWithDebInfo (default: RelWithDebInfo)
  --build-dir PATH    Temporary build directory (default: auto in /tmp)
  --keep-src          Keep source directory after install
  -h, --help          Show this help

Examples:
  $(basename "$0")                                          # scylladb -> /usr/local/scylladb
  $(basename "$0") --driver cassandra                       # cassandra -> /usr/local/cassandra
  $(basename "$0") --driver cassandra --prefix \$HOME/.local # -> ~/.local/cassandra
  $(basename "$0") --layout flat --prefix /opt/scylladb     # legacy flat install

After install, point the PHP extension at it via:
  cmake --preset DebugPHP8.4NTS -DCPP_DRIVER_PREFIX=<prefix>/<driver>

EOF
}

die() { echo "ERROR: $*" >&2; exit 1; }

# sudo-if-needed wrapper for Linux package managers
SUDO=""
maybe_sudo() {
    if [[ $EUID -ne 0 ]] && command -v sudo >/dev/null 2>&1; then
        SUDO="sudo"
    fi
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --driver)     DRIVER="${2:?--driver requires a value}"; shift 2 ;;
        --prefix)     INSTALL_PREFIX="${2:?--prefix requires a value}"; shift 2 ;;
        --layout)     LAYOUT="${2:?--layout requires a value}"; shift 2 ;;
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
[[ "$LAYOUT" == "per-driver" || "$LAYOUT" == "flat" ]] \
    || die "Invalid --layout '$LAYOUT'. Must be 'per-driver' or 'flat'."

# Default prefix root
: "${INSTALL_PREFIX:=/usr/local}"

# Resolve final install path
if [[ "$LAYOUT" == "per-driver" ]]; then
    FINAL_PREFIX="$INSTALL_PREFIX/$DRIVER"
else
    FINAL_PREFIX="$INSTALL_PREFIX"
fi

command -v cmake >/dev/null 2>&1 || die "cmake not found in PATH"
command -v git   >/dev/null 2>&1 || die "git not found in PATH"
command -v ninja >/dev/null 2>&1 || die "ninja not found in PATH"

case "$DRIVER" in
    scylladb)  GIT_REPO="https://github.com/scylladb/cpp-driver.git" ;;
    cassandra) GIT_REPO="https://github.com/apache/cassandra-cpp-driver.git" ;;
esac

# Populated by install_system_deps on macOS so we can pass them to cmake.
OPENSSL_ROOT=""
LIBSSH2_ROOT=""
LIBUV_ROOT=""
ZLIB_ROOT=""

install_system_deps() {
    case "$(uname -s)" in
        Darwin)
            echo "==> Installing build deps (macOS)"
            command -v brew >/dev/null 2>&1 || die "Homebrew not found. Install from https://brew.sh"
            brew install cmake ninja pkg-config openssl@3 libssh2 libuv zlib

            # Capture keg-only formula prefixes so cmake can find them.
            OPENSSL_ROOT="$(brew --prefix openssl@3 2>/dev/null || true)"
            LIBSSH2_ROOT="$(brew --prefix libssh2 2>/dev/null || true)"
            LIBUV_ROOT="$(brew --prefix libuv 2>/dev/null || true)"
            ZLIB_ROOT="$(brew --prefix zlib 2>/dev/null || true)"
            ;;
        Linux)
            maybe_sudo
            # shellcheck source=/dev/null
            . /etc/os-release 2>/dev/null || return
            case "${ID:-}" in
                fedora)
                    echo "==> Installing build deps (Fedora)"
                    $SUDO dnf install -y cmake pkg-config gcc ninja-build \
                        openssl-devel openssl-devel-engine libssh2-devel libuv-devel zlib-devel
                    ;;
                ubuntu|debian)
                    echo "==> Installing build deps (Ubuntu/Debian)"
                    $SUDO apt-get update
                    $SUDO apt-get install -y pkg-config build-essential cmake ninja-build \
                        libssl-dev libssh2-1-dev libuv1-dev zlib1g-dev
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

# timerfd is Linux-only — must be OFF on macOS.
TIMERFD="ON"
[[ "$(uname -s)" == "Darwin" ]] && TIMERFD="OFF"

# Build up cmake hint args for keg-only / non-default deps.
CMAKE_DEP_HINTS=()
[[ -n "$OPENSSL_ROOT"  ]] && CMAKE_DEP_HINTS+=( "-DOPENSSL_ROOT_DIR=$OPENSSL_ROOT" )
[[ -n "$LIBSSH2_ROOT"  ]] && CMAKE_DEP_HINTS+=( "-DLIBSSH2_ROOT_DIR=$LIBSSH2_ROOT" )
[[ -n "$LIBUV_ROOT"    ]] && CMAKE_DEP_HINTS+=( "-DLIBUV_ROOT_DIR=$LIBUV_ROOT" )
[[ -n "$ZLIB_ROOT"     ]] && CMAKE_DEP_HINTS+=( "-DZLIB_ROOT=$ZLIB_ROOT" )

# Compose CMAKE_PREFIX_PATH from the keg-only roots (helps modern find_package).
PREFIX_PATH_ENTRIES=()
for r in "$OPENSSL_ROOT" "$LIBSSH2_ROOT" "$LIBUV_ROOT" "$ZLIB_ROOT"; do
    [[ -n "$r" ]] && PREFIX_PATH_ENTRIES+=( "$r" )
done
if [[ ${#PREFIX_PATH_ENTRIES[@]} -gt 0 ]]; then
    PREFIX_PATH_JOINED="$(IFS=';'; echo "${PREFIX_PATH_ENTRIES[*]}")"
    CMAKE_DEP_HINTS+=( "-DCMAKE_PREFIX_PATH=$PREFIX_PATH_JOINED" )
fi

echo "==> Configuring (prefix=$FINAL_PREFIX, build=$BUILD_TYPE, layout=$LAYOUT)"
cmake -G Ninja -B "$SRC_DIR/build" -S "$SRC_DIR" \
    -DCMAKE_C_FLAGS="-fPIC" \
    -DCMAKE_CXX_FLAGS="-fPIC -Wno-error=redundant-move" \
    -DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DCMAKE_INSTALL_PREFIX="$FINAL_PREFIX" \
    -DCASS_CPP_STANDARD=17 \
    -DCASS_BUILD_STATIC=ON \
    -DCASS_BUILD_SHARED=ON \
    -DCASS_USE_STD_ATOMIC=ON \
    -DCASS_USE_STATIC_LIBS=ON \
    -DCASS_USE_TIMERFD="$TIMERFD" \
    -DCASS_USE_LIBSSH2=ON \
    -DCASS_USE_ZLIB=ON \
    ${CCACHE_DIR:+-DCMAKE_CXX_COMPILER_LAUNCHER=ccache} \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=OFF \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    "${CMAKE_DEP_HINTS[@]}"

echo "==> Building (jobs=$NPROC)"
cmake --build "$SRC_DIR/build" --parallel "$NPROC"

# Use sudo for install if writing under /usr or /opt and we're not root.
INSTALL_SUDO=""
if [[ "$(uname -s)" != "Darwin" && $EUID -ne 0 ]]; then
    case "$FINAL_PREFIX" in
        /usr/*|/opt/*) command -v sudo >/dev/null 2>&1 && INSTALL_SUDO="sudo" ;;
    esac
fi

echo "==> Installing to $FINAL_PREFIX"
$INSTALL_SUDO cmake --install "$SRC_DIR/build"

cat <<EOF

==> $DRIVER cpp-driver installed to $FINAL_PREFIX

Next:
  cmake --preset DebugPHP8.4NTS -DCPP_DRIVER_PREFIX="$FINAL_PREFIX"
  cmake --build out/DebugPHP8.4NTS

EOF
