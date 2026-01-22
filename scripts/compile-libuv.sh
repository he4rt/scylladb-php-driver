#!/bin/bash

LIBUV_VERSION="v1.50.0"
LIBUV_REPO="https://github.com/libuv/libuv.git"
CURRENT_DIR="$(pwd)"
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
GIT_OUTPUT="$PROJECT_ROOT/third-party/libuv"
INSTALL_PREFIX="$PROJECT_ROOT/third-party/libuv-install"

is_macos() {
  local value

  value="$(uname -s)"

  if [ "$value" = "Darwin" ]; then
    return 0
  fi

  return 1
}

get_cpu_count() {
  if is_macos; then
    sysctl -n hw.ncpu
  else
    nproc
  fi
}

if is_macos; then
  if ! command -v brew &> /dev/null; then
    echo "Homebrew is required but not installed. Please install it from https://brew.sh"
    exit 1
  fi

  brew install cmake ninja || exit 1
fi

git clone --depth=1 "$LIBUV_REPO" "$GIT_OUTPUT"

cd "$GIT_OUTPUT" || exit 1

git fetch --tags

git checkout -b $LIBUV_VERSION tags/$LIBUV_VERSION

DEBUG_FLAGS="-g -ggdb -g3 -gdwarf-5 -fno-omit-frame-pointer"

CFLAGS="-fPIC $DEBUG_FLAGS" cmake -G Ninja -B build \
    -DBUILD_TESTING=OFF \
    -DBUILD_BENCHMARKS=OFF \
    -DLIBUV_BUILD_SHARED=ON \
    -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
    -DCMAKE_BUILD_TYPE="Debug"

CFLAGS="-fPIC $DEBUG_FLAGS" cmake --build build --parallel "$(get_cpu_count)"
cmake --install build

cd "$CURRENT_DIR" || exit 1
