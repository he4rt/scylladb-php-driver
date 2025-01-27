#!/bin/bash

DRIVER=${1:-"scylladb"}
VERSION=${2:-"master"}

rm -rf out

phpize --clean
phpize

if [[ "$DRIVER" == "cassandra" ]]; then
    ./configure \
        --enable-lto \
        --enable-avx \
        --enable-libuv-static \
        --enable-driver-static \
        --enable-libcassandra \
        --with-version="$VERSION"
else
    ./configure \
        --enable-lto \
        --enable-avx \
        --enable-libuv-static \
        --enable-driver-static \
        --with-version="$VERSION"
fi

make "-j$(nproc)" || exit 1
make install
cp "$(php-config --extension-dir)/cassandra.so" .
