#!/usr/bin/env bash
# Clean build artifacts and caches
# Usage: ./scripts/clean.sh [all|extension|zendcpp|deps]

set -euo pipefail

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.."; pwd)"
cd "$PROJECT_ROOT"

CLEAN_TYPE="${1:-all}"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

log_info() {
    echo -e "${GREEN}[CLEAN]${NC} $1"
}

log_warn() {
    echo -e "${YELLOW}[WARN]${NC} $1"
}

clean_extension() {
    log_info "Cleaning extension build artifacts..."

    if [ -f "Makefile" ]; then
        make clean 2>/dev/null || true
    fi

    phpize --clean 2>/dev/null || true

    rm -rf \
        .libs \
        *.lo \
        *.la \
        *.o \
        modules/ \
        Makefile \
        Makefile.fragments \
        Makefile.objects \
        config.status \
        config.log \
        config.nice \
        configure \
        libtool \
        autom4te.cache/ \
        build/ \
        include/ \
        run-tests.php \
        cassandra.so \
        .deps/ \
        2>/dev/null || true

    log_info "Extension cleaned"
}

clean_zendcpp() {
    log_info "Cleaning ZendCPP test build artifacts..."

    cd "$PROJECT_ROOT/ZendCPP/tests"

    if [ -f "Makefile" ]; then
        make clean 2>/dev/null || true
    fi

    phpize --clean 2>/dev/null || true

    rm -rf \
        .libs \
        *.lo \
        *.la \
        *.o \
        modules/ \
        Makefile \
        config.status \
        config.log \
        config.nice \
        configure \
        libtool \
        autom4te.cache/ \
        build/ \
        .deps/ \
        2>/dev/null || true

    cd "$PROJECT_ROOT"
    log_info "ZendCPP cleaned"
}

clean_deps() {
    log_info "Cleaning dependency builds..."

    log_warn "This will remove all built dependencies. They will need to be rebuilt."
    read -p "Continue? (y/N) " -n 1 -r
    echo

    if [[ ! $REPLY =~ ^[Yy]$ ]]; then
        log_info "Cancelled"
        return 0
    fi

    rm -rf \
        third-party/libuv-src/ \
        third-party/libuv-install/ \
        third-party/scylladb-cpp-src/ \
        third-party/scylladb-driver-install/ \
        third-party/cassandra-cpp-src/ \
        third-party/datastax-driver-install/ \
        2>/dev/null || true

    log_info "Dependencies cleaned"
}

clean_cmake() {
    log_info "Cleaning CMake build artifacts..."

    rm -rf \
        out/ \
        cmake-build-*/ \
        CMakeFiles/ \
        CMakeCache.txt \
        cmake_install.cmake \
        2>/dev/null || true

    log_info "CMake cleaned"
}

clean_composer() {
    log_info "Cleaning Composer dependencies..."

    rm -rf \
        vendor/ \
        tests/vendor/ \
        composer.lock \
        tests/composer.lock \
        .phpunit.result.cache \
        tests/.phpunit.result.cache \
        2>/dev/null || true

    log_info "Composer cleaned"
}

clean_all() {
    log_info "Cleaning everything..."
    clean_extension
    clean_zendcpp
    clean_cmake
    clean_composer
    clean_deps
    log_info "All clean!"
}

# Main execution
case "$CLEAN_TYPE" in
    all)
        clean_all
        ;;
    extension)
        clean_extension
        ;;
    zendcpp)
        clean_zendcpp
        ;;
    deps|dependencies)
        clean_deps
        ;;
    cmake)
        clean_cmake
        ;;
    composer)
        clean_composer
        ;;
    *)
        echo "Usage: $0 [all|extension|zendcpp|deps|cmake|composer]"
        echo ""
        echo "Options:"
        echo "  all        - Clean everything (extension, zendcpp, cmake, composer, deps)"
        echo "  extension  - Clean extension build artifacts"
        echo "  zendcpp    - Clean ZendCPP test build artifacts"
        echo "  deps       - Clean dependency builds (interactive)"
        echo "  cmake      - Clean CMake build artifacts"
        echo "  composer   - Clean Composer dependencies"
        exit 1
        ;;
esac
