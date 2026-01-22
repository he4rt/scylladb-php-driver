#!/usr/bin/env bash
# Check if third-party dependencies are built

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "==================================="
echo "Third-Party Dependencies Check"
echo "==================================="
echo ""

check_lib() {
    local name=$1
    local path=$2
    local pc_file=$3

    echo "Checking $name..."
    if [ -f "$path/$pc_file" ]; then
        echo "  ✓ Found: $path/$pc_file"
        version=$(pkg-config --modversion --define-variable=prefix="$(dirname "$(dirname "$path")")" "$(basename "$pc_file" .pc)" 2>/dev/null || echo "unknown")
        echo "  Version: $version"
    else
        echo "  ✗ NOT FOUND: $path/$pc_file"
        echo "  To build, run: ./scripts/compile-$(echo "$name" | tr '[:upper:]' '[:lower:]' | tr ' ' '-').sh"
    fi
    echo ""
}

check_php() {
    echo "Checking PHP..."

    # Check system php-config
    if command -v php-config &> /dev/null; then
        echo "  ✓ System php-config found: $(which php-config)"
        echo "  Version: $(php-config --version)"
        echo "  Include dir: $(php-config --include-dir)"
    else
        echo "  ✗ System php-config not found"
    fi

    # Check local third-party PHP
    local php_found=false
    if [ -d "$PROJECT_ROOT/third-party/php" ]; then
        for php_dir in "$PROJECT_ROOT/third-party/php"/*; do
            if [ -f "$php_dir/bin/php-config" ]; then
                echo "  ✓ Local PHP found: $php_dir/bin/php-config"
                version=$("$php_dir/bin/php-config" --version 2>/dev/null || echo "unknown")
                echo "  Version: $version"
                php_found=true
            fi
        done
    fi

    if [ "$php_found" = false ] && ! command -v php-config &> /dev/null; then
        echo "  Note: No local PHP builds found in third-party/php/"
        echo "  Note: System php-config will be used if available"
    fi
    echo ""
}

# Check PHP first
check_php

# Check libuv
check_lib "libuv" \
    "$PROJECT_ROOT/third-party/libuv-install/lib/pkgconfig" \
    "libuv.pc"

# Check ScyllaDB driver
check_lib "ScyllaDB C++ Driver" \
    "$PROJECT_ROOT/third-party/scylladb-driver-install/lib/pkgconfig" \
    "scylla-cpp-driver.pc"

# Check Cassandra driver
check_lib "Cassandra C++ Driver" \
    "$PROJECT_ROOT/third-party/datastax-driver-install/lib/pkgconfig" \
    "cassandra.pc"

echo "==================================="
echo ""
echo "Summary:"
echo "  PHP: System php-config is OK for development and CI"
echo "  Libraries: Need to be built for local development"
echo ""
echo "  To build dependencies locally:"
echo "    ./scripts/compile-libuv.sh"
echo "    ./scripts/compile-cpp-driver.sh scylladb"
echo ""
echo "  Or let CI build them via the build-dependencies job."
echo ""
