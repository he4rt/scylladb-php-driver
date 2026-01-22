#!/bin/bash

SCYLLA_OR_CASSANDRA=${1:-"scylladb"}
CURRENT_DIR="$(pwd)"
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

GIT_REPO="https://github.com/$SCYLLA_OR_CASSANDRA/cpp-driver.git"
GIT_OUTPUT="$PROJECT_ROOT/third-party/$SCYLLA_OR_CASSANDRA-driver"
INSTALL_PREFIX="$PROJECT_ROOT/third-party/$SCYLLA_OR_CASSANDRA-driver-install"

is_linux() {
  local value

  value="$(uname -s)"

  if [ "$value" = "Linux" ]; then
    return 0
  fi

  return 1
}

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

if is_linux; then
  . /etc/os-release

  if [[ "$NAME" == "Fedora Linux" ]]; then
    dnf install \
      cmake \
      pkg-config \
      gcc \
      ninja-build \
      openssl-devel \
      openssl-devel-engine || exit 1
  fi

  if [[ "$NAME" == "Ubuntu" ]]; then
    apt-get install \
      pkg-config \
      build-essential \
      libssl-dev || exit 1
  fi
elif is_macos; then
  if ! command -v brew &> /dev/null; then
    echo "Homebrew is required but not installed. Please install it from https://brew.sh"
    exit 1
  fi

  brew install \
    cmake \
    pkg-config \
    ninja \
    openssl || exit 1
fi

git clone --depth 1 "$GIT_REPO" "$GIT_OUTPUT"

cd "$GIT_OUTPUT" || exit 1

# Patch CMakeLists.txt to fix cmake_minimum_required version
sed -i.bak 's/cmake_minimum_required(VERSION [0-9.]*)/cmake_minimum_required(VERSION 3.5)/' CMakeLists.txt

# Patch CMakeLists.txt to support AppleClang on macOS
if is_macos; then
  sed -i '' 's/message(FATAL_ERROR "Unsupported compiler: ${CMAKE_CXX_COMPILER_ID}")/message(STATUS "Using compiler: ${CMAKE_CXX_COMPILER_ID}")/' CMakeLists.txt
fi

DEBUG_FLAGS="-g -ggdb -g3 -gdwarf-5 -fno-omit-frame-pointer"

export LIBUV_INSTALL_PATH="$PROJECT_ROOT/third-party/libuv-install"

CFLAGS="-fPIC $DEBUG_FLAGS" CXXFLAGS="-fPIC $DEBUG_FLAGS -Wno-error=redundant-move" cmake -G Ninja -B build \
  -DCASS_CPP_STANDARD=17 \
  -DCASS_BUILD_STATIC=ON \
  -DCASS_BUILD_SHARED=ON \
  -DCASS_USE_STD_ATOMIC=ON \
  -DCASS_USE_STATIC_LIBS=ON \
  -DCASS_USE_TIMERFD=ON \
  -DCASS_USE_LIBSSH2=ON \
  -DCASS_USE_ZLIB=ON \
  -DLIBUV_ROOT_DIR="$LIBUV_INSTALL_PATH" \
  -DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX" \
  -DCMAKE_EXPORT_COMPILE_COMMANDS="OFF" \
  -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
  -DCMAKE_BUILD_TYPE="Debug"

CFLAGS="-fPIC $DEBUG_FLAGS" CXXFLAGS="-fPIC $DEBUG_FLAGS -Wno-error=redundant-move" cmake --build build --parallel "$(get_cpu_count)"
cmake --install build

cd "$CURRENT_DIR" || exit 1
