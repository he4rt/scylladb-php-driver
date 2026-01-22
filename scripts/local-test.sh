#!/usr/bin/env bash
# Local testing script that mimics GitHub Actions workflow
# Usage: ./scripts/local-test.sh [zendcpp|extension|all] [php-version] [driver]

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.."; pwd)"
cd "$PROJECT_ROOT"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Configuration
TEST_TYPE="${1:-all}"
PHP_VERSION="${2:-8.4}"
DRIVER="${3:-scylladb}"
THREADING="${4:-nts}"

THIRD_PARTY_DIR="$PROJECT_ROOT/third-party"
LIBUV_INSTALL="$THIRD_PARTY_DIR/libuv-install"
SCYLLADB_DRIVER_INSTALL="$THIRD_PARTY_DIR/scylladb-driver-install"
CASSANDRA_DRIVER_INSTALL="$THIRD_PARTY_DIR/datastax-driver-install"

log_info() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

log_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

check_command() {
    if ! command -v "$1" &> /dev/null; then
        log_error "$1 is required but not installed."
        return 1
    fi
}

# Check required tools
log_info "Checking required tools..."
check_command php
check_command phpize
check_command cmake
check_command ninja
check_command pkg-config

# Build dependencies
build_libuv() {
    if [ -d "$LIBUV_INSTALL" ]; then
        log_info "libuv already built, skipping..."
        return 0
    fi

    log_info "Building libuv..."
    mkdir -p "$THIRD_PARTY_DIR"

    if [ ! -d "$THIRD_PARTY_DIR/libuv-src" ]; then
        git clone --depth 1 --branch v1.50.0 https://github.com/libuv/libuv.git "$THIRD_PARTY_DIR/libuv-src"
    fi

    cd "$THIRD_PARTY_DIR/libuv-src"
    cmake -G Ninja -B build \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DBUILD_TESTING=OFF \
        -DBUILD_BENCHMARKS=OFF \
        -DLIBUV_BUILD_SHARED=ON \
        -DCMAKE_INSTALL_PREFIX="$LIBUV_INSTALL"
    cmake --build build
    cmake --install build
    cd "$PROJECT_ROOT"
}

build_scylladb_driver() {
    if [ -d "$SCYLLADB_DRIVER_INSTALL" ]; then
        log_info "ScyllaDB driver already built, skipping..."
        return 0
    fi

    log_info "Building ScyllaDB C++ driver..."

    if [ ! -d "$THIRD_PARTY_DIR/scylladb-cpp-src" ]; then
        git clone --depth 1 https://github.com/scylladb/cpp-driver.git "$THIRD_PARTY_DIR/scylladb-cpp-src"
    fi

    cd "$THIRD_PARTY_DIR/scylladb-cpp-src"
    cmake -G Ninja -B build \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCASS_CPP_STANDARD=17 \
        -DCASS_BUILD_STATIC=ON \
        -DCASS_BUILD_SHARED=ON \
        -DCASS_USE_STD_ATOMIC=ON \
        -DCASS_USE_STATIC_LIBS=ON \
        -DCASS_USE_TIMERFD=ON \
        -DCASS_USE_LIBSSH2=ON \
        -DCASS_USE_ZLIB=ON \
        -DCASS_BUILD_TESTS=OFF \
        -DCASS_BUILD_EXAMPLES=OFF \
        -DCMAKE_INSTALL_PREFIX="$SCYLLADB_DRIVER_INSTALL" \
        -DCMAKE_PREFIX_PATH="$LIBUV_INSTALL"
    cmake --build build
    cmake --install build
    cd "$PROJECT_ROOT"
}

build_cassandra_driver() {
    if [ -d "$CASSANDRA_DRIVER_INSTALL" ]; then
        log_info "Cassandra driver already built, skipping..."
        return 0
    fi

    log_info "Building Cassandra C++ driver..."

    if [ ! -d "$THIRD_PARTY_DIR/cassandra-cpp-src" ]; then
        git clone --depth 1 https://github.com/datastax/cpp-driver.git "$THIRD_PARTY_DIR/cassandra-cpp-src"
    fi

    cd "$THIRD_PARTY_DIR/cassandra-cpp-src"
    cmake -G Ninja -B build \
        -DCMAKE_BUILD_TYPE=RelWithDebInfo \
        -DCASS_CPP_STANDARD=17 \
        -DCASS_BUILD_STATIC=ON \
        -DCASS_BUILD_SHARED=ON \
        -DCASS_USE_STD_ATOMIC=ON \
        -DCASS_USE_TIMERFD=ON \
        -DCASS_USE_LIBSSH2=ON \
        -DCASS_USE_STATIC_LIBS=ON \
        -DCASS_USE_ZLIB=ON \
        -DCASS_BUILD_TESTS=OFF \
        -DCASS_BUILD_EXAMPLES=OFF \
        -DCASS_BUILD_UNIT_TESTS=OFF \
        -DCASS_BUILD_INTEGRATION_TESTS=OFF \
        -DCMAKE_INSTALL_PREFIX="$CASSANDRA_DRIVER_INSTALL" \
        -DCMAKE_PREFIX_PATH="$LIBUV_INSTALL"
    cmake --build build
    cmake --install build
    cd "$PROJECT_ROOT"
}

test_zendcpp() {
    log_info "Testing ZendCPP..."

    cd "$PROJECT_ROOT/ZendCPP/tests"

    # Clean previous builds
    if [ -f "Makefile" ]; then
        make clean || true
    fi
    phpize --clean || true

    # Build
    phpize
    ./configure --enable-zendcpp_test
    make -j$(nproc)

    # Run tests
    log_info "Running ZendCPP tests..."
    php -d extension=modules/zendcpp_test.so run_tests.php

    cd "$PROJECT_ROOT"
}

test_extension() {
    log_info "Testing PHP Extension with $DRIVER driver..."

    # Setup environment
    export PKG_CONFIG_PATH="$LIBUV_INSTALL/lib/pkgconfig:$SCYLLADB_DRIVER_INSTALL/lib/pkgconfig:$CASSANDRA_DRIVER_INSTALL/lib/pkgconfig:$PKG_CONFIG_PATH"
    export LD_LIBRARY_PATH="$LIBUV_INSTALL/lib:$SCYLLADB_DRIVER_INSTALL/lib:$CASSANDRA_DRIVER_INSTALL/lib:$LD_LIBRARY_PATH"

    cd "$PROJECT_ROOT"

    # Clean previous builds
    if [ -f "Makefile" ]; then
        make clean || true
    fi
    phpize --clean || true

    # Build
    phpize

    if [ "$DRIVER" = "cassandra" ]; then
        ./configure \
            --enable-lto \
            --enable-avx \
            --enable-libuv-static \
            --enable-driver-static \
            --enable-libcassandra
    else
        ./configure \
            --enable-lto \
            --enable-avx \
            --enable-libuv-static \
            --enable-driver-static
    fi

    make -j$(nproc)
    make install

    # Verify extension loads
    log_info "Verifying extension loads..."
    php -d extension=cassandra.so -m | grep cassandra

    # Install test dependencies
    cd "$PROJECT_ROOT/tests"
    if [ ! -d "vendor" ]; then
        log_info "Installing test dependencies..."
        composer install --no-interaction --no-progress --prefer-dist
    fi

    # Run tests
    log_info "Running extension tests..."
    php -d extension=cassandra.so ./vendor/bin/pest \
        --colors=always \
        --fail-on-risky \
        --fail-on-warning

    cd "$PROJECT_ROOT"
}

# Main execution
log_info "Starting local tests..."
log_info "Test type: $TEST_TYPE"
log_info "PHP version: $(php -v | head -n 1)"
log_info "Driver: $DRIVER"

# Build dependencies
log_info "Building dependencies..."
build_libuv

if [ "$DRIVER" = "cassandra" ]; then
    build_cassandra_driver
else
    build_scylladb_driver
fi

# Run tests
case "$TEST_TYPE" in
    zendcpp)
        test_zendcpp
        ;;
    extension)
        test_extension
        ;;
    all)
        test_zendcpp
        test_extension
        ;;
    *)
        log_error "Invalid test type: $TEST_TYPE"
        log_error "Usage: $0 [zendcpp|extension|all] [php-version] [driver]"
        exit 1
        ;;
esac

log_info "All tests completed successfully!"
