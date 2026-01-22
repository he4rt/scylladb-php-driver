#!/bin/bash

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.."; pwd)"

DRIVER=${1:-"scylladb"}
VERSION=${2:-"master"}
PHP_VERSION=${3:-"8.2"}

PHP_DIR=${4:-"$PROJECT_ROOT/third-party/php/$PHP_VERSION-debug-nts/bin"}

rm -rf out

"$PHP_DIR/phpize" --clean
"$PHP_DIR/phpize"

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
cp "$("$PHP_DIR/php-config" --extension-dir)/cassandra.so" .
