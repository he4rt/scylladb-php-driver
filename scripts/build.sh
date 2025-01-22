#!/bin/bash

DRIVER=${1:-"scylladb"}
VERSION=${2:-"master"}

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
mv "$(php-config --extension-dir)/cassandra.so" .
