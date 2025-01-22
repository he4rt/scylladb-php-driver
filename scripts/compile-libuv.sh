#!/bin/bash

LIBUV_VERSION="v1.50.0"
LIBUV_REPO="https://github.com/libuv/libuv.git"
CURRENT_DIR="$(pwd)"

git clone --depth=1 "$LIBUV_REPO" /opt/libuv

cd /opt/libuv || exit 1

git fetch --tags

git checkout -b $LIBUV_VERSION tags/$LIBUV_VERSION

LDFLAGS="-flto" CFLAGS="-fPIC" cmake -G Ninja -B build \
    -DBUILD_TESTING=OFF \
    -DBUILD_BENCHMARKS=OFF \
    -DLIBUV_BUILD_SHARED=OFF \
    -DCMAKE_CXX_COMPILER_LAUNCHER=ccache \
    -DCMAKE_BUILD_TYPE="RelWithInfo"

LDFLAGS="-flto" CFLAGS="-fPIC" cmake --build build

rm -rf build

cd "$CURRENT_DIR" || exit 1
