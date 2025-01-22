#!/bin/bash

SCYLLA_OR_CASSANDRA=${1:-"scylladb"}
CURRENT_DIR="$(pwd)"

GIT_REPO="https://github.com/$SCYLLA_OR_CASSANDRA/cpp-driver.git"
GIT_OUTPUT="/opt/$SCYLLA_OR_CASSANDRA-driver"

is_linux() {
  local value

  value="$(uname -s)"

  if [ "$value" = "Linux" ]; then
    return 0
  fi

  return 1
}

if ! is_linux; then
  echo "This script is for Linux only"
  exit 1
fi

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

git clone --depth 1 "$GIT_REPO" "$GIT_OUTPUT"

cd "$GIT_OUTPUT" || exit 1

CFLAGS="-fPIC" CXXFLAGS="-fPIC -Wno-error=redundant-move" LDFLAGS="-flto" cmake -G Ninja -B build \
  -DCASS_CPP_STANDARD=17 \
  -DCASS_BUILD_STATIC=ON \
  -DCASS_BUILD_SHARED=ON \
  -DCASS_USE_STD_ATOMIC=ON \
  -DCASS_USE_STATIC_LIBS=ON \
  -DCASS_USE_TIMERFD=ON \
  -DCASS_USE_LIBSSH2=ON \
  -DCASS_USE_ZLIB=ON \
  -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
  -DCMAKE_EXPORT_COMPILE_COMMANDS="OFF" \
  -DCMAKE_BUILD_TYPE="RelWithInfo"

CFLAGS="-fPIC" CXXFLAGS="-fPIC -Wno-error=redundant-move" LDFLAGS="-flto" cmake --build build

cd .. || exit

rm -rf build

cd "$CURRENT_DIR" || exit 1
