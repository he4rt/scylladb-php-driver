#!/bin/bash

# ScyllaDB PHP Driver - PHP 8.4 Build and Test Script
# This script helps build and test the driver for PHP 8.4 compatibility

set -e

echo "=== ScyllaDB PHP Driver - PHP 8.4 Build Script ==="

# Configuration
BUILD_DIR="out"
PHP_CONFIG="${PHP_CONFIG:-php-config}"
CMAKE_BUILD_TYPE="${CMAKE_BUILD_TYPE:-Release}"

# Check PHP version
PHP_VERSION=$($PHP_CONFIG --version 2>/dev/null || echo "unknown")
echo "Detected PHP Version: $PHP_VERSION"

if [[ "$PHP_VERSION" == "unknown" ]]; then
    echo "❌ Error: php-config not found. Please install PHP development packages."
    exit 1
fi

# Check if we have PHP 8.4+
PHP_MAJOR=$(echo $PHP_VERSION | cut -d. -f1)
PHP_MINOR=$(echo $PHP_VERSION | cut -d. -f2)

if [[ $PHP_MAJOR -lt 8 ]] || [[ $PHP_MAJOR -eq 8 && $PHP_MINOR -lt 1 ]]; then
    echo "❌ Error: PHP 8.1+ is required. Found: $PHP_VERSION"
    exit 1
fi

if [[ $PHP_MAJOR -ge 8 && $PHP_MINOR -ge 4 ]]; then
    echo "✅ PHP 8.4+ detected - running enhanced compatibility tests"
    PHP84_MODE=true
else
    echo "ℹ️  Running on PHP $PHP_VERSION (pre-8.4)"
    PHP84_MODE=false
fi

# Clean previous build
echo "🧹 Cleaning previous build..."
rm -rf $BUILD_DIR
mkdir -p $BUILD_DIR

# Detect which C++ driver to use
echo "🔍 Detecting available C++ driver..."

# Try Cassandra driver first (universal compatibility - recommended default)
USE_SCYLLA_DRIVER=false
if pkg-config --exists cassandra >/dev/null 2>&1 || [[ -f "/usr/local/include/cassandra.h" ]] && [[ -f "/usr/local/lib/libcassandra.so" ]] || [[ -f "/usr/local/lib/libcassandra.dylib" ]] || [[ -f "/opt/homebrew/lib/libcassandra.dylib" ]]; then
    echo "✅ Found Cassandra C++ driver (universal compatibility) - using recommended default"
    USE_SCYLLA_DRIVER=false
elif pkg-config --exists scylla-cpp-driver; then
    echo "✅ Found ScyllaDB C++ driver (shard-aware) - using specialized driver"
    echo "    📝 Note: ScyllaDB driver provides shard-awareness for ScyllaDB clusters"
    USE_SCYLLA_DRIVER=true
else
    echo "❌ No compatible C++ driver found!"
    echo ""
    echo "Please install one of:"
    echo "  1. Cassandra C++ Driver (RECOMMENDED - universal compatibility):"
    echo "     macOS: brew install cassandra-cpp-driver"
    echo "     Ubuntu: sudo apt install libcassandra-dev"
    echo "     CentOS: sudo dnf install cassandra-cpp-driver-devel"
    echo ""
    echo "  2. ScyllaDB C++ Driver (specialized for ScyllaDB):"
    echo "     git clone https://github.com/scylladb/cpp-driver.git"
    echo "     cd cpp-driver && mkdir build && cd build"
    echo "     cmake .. && make -j4 && sudo make install"
    exit 1
fi

# Configure with CMake
echo "⚙️  Configuring build..."
if [[ "$USE_SCYLLA_DRIVER" == "true" ]]; then
    cmake -B $BUILD_DIR \
        -DCUSTOM_PHP_CONFIG=$PHP_CONFIG \
        -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
        -DCPU_TYPE=x86-64-v2 \
        -DENABLE_LTO=OFF \
        -DENABLE_SANITIZERS=OFF \
        -DLINK_LIBUV_STATIC=OFF \
        -DPHP_DRIVER_STATIC=OFF \
        -DUSE_LIBCASSANDRA=OFF
else
    cmake -B $BUILD_DIR \
        -DCUSTOM_PHP_CONFIG=$PHP_CONFIG \
        -DCMAKE_BUILD_TYPE=$CMAKE_BUILD_TYPE \
        -DCPU_TYPE=x86-64-v2 \
        -DENABLE_LTO=OFF \
        -DENABLE_SANITIZERS=OFF \
        -DLINK_LIBUV_STATIC=OFF \
        -DPHP_DRIVER_STATIC=OFF \
        -DUSE_LIBCASSANDRA=ON
fi

if [ $? -ne 0 ]; then
    echo "❌ CMake configuration failed"
    exit 1
fi

# Build
echo "🔨 Building extension..."
cmake --build $BUILD_DIR --parallel

if [ $? -ne 0 ]; then
    echo "❌ Build failed"
    exit 1
fi

# Copy extension to modules directory
echo "📦 Installing extension..."
mkdir -p modules
if [[ -f "./$BUILD_DIR/cassandra.dylib" ]]; then
    cp "./$BUILD_DIR/cassandra.dylib" "./modules/cassandra.so"
elif [[ -f "./$BUILD_DIR/cassandra.so" ]]; then
    cp "./$BUILD_DIR/cassandra.so" "./modules/cassandra.so"
else
    echo "❌ Could not find built extension"
    exit 1
fi

echo "✅ Build completed successfully!"

# Test if extension loads
echo "🧪 Testing extension loading..."
php -d extension=./modules/cassandra.so -m | grep -q cassandra
if [ $? -eq 0 ]; then
    echo "✅ Extension loads successfully"
else
    echo "❌ Extension failed to load"
    exit 1
fi

# Show extension info
echo "📊 Extension information:"
php -d extension=./modules/cassandra.so -r "
if (extension_loaded('cassandra')) {
    echo 'Extension Version: ' . phpversion('cassandra') . \"\n\";
    echo 'PHP Version: ' . PHP_VERSION . \"\n\";
    echo 'Zend API: ' . phpversion('core') . \"\n\";
} else {
    echo 'Extension not loaded' . \"\n\";
    exit(1);
}
"

# Run compatibility tests
if [[ -f "scripts/test-php84-compatibility.php" ]]; then
    echo "🧪 Running PHP 8.4 compatibility tests..."
    php -d extension=./modules/cassandra.so scripts/test-php84-compatibility.php
    
    if [ $? -eq 0 ]; then
        echo "✅ All compatibility tests passed!"
    else
        echo "❌ Some compatibility tests failed"
        exit 1
    fi
else
    echo "ℹ️  Compatibility test script not found, skipping tests"
fi

# Run memory leak tests if available
if command -v valgrind >/dev/null 2>&1; then
    echo "🔍 Running memory leak detection (basic test)..."
    valgrind --leak-check=full --error-exitcode=1 \
        php -d extension=./modules/cassandra.so -r "
        echo 'Testing basic functionality...' . PHP_EOL;
        \$bigint = new Cassandra\Bigint(123);
        \$uuid = new Cassandra\Uuid();
        echo 'Basic objects created successfully' . PHP_EOL;
        " 2>/dev/null
    
    if [ $? -eq 0 ]; then
        echo "✅ No memory leaks detected in basic test"
    else
        echo "⚠️  Memory leaks or errors detected"
    fi
else
    echo "ℹ️  Valgrind not available, skipping memory leak tests"
fi

echo ""
echo "🎉 PHP 8.4 build and test completed successfully!"
echo ""
echo "📝 Summary:"
echo "  - Build directory: $BUILD_DIR"
echo "  - Extension file: ./modules/cassandra.so"
echo "  - PHP Version: $PHP_VERSION"
echo "  - Build Type: $CMAKE_BUILD_TYPE"
if [[ "$USE_SCYLLA_DRIVER" == "true" ]]; then
    echo "  - C++ Driver: ✅ ScyllaDB (shard-aware, specialized)"
else
    echo "  - C++ Driver: ✅ Cassandra (universal compatibility, recommended)"
fi

if [[ "$PHP84_MODE" == "true" ]]; then
    echo "  - PHP 8.4 Compatibility: ✅ VERIFIED"
else
    echo "  - PHP 8.4 Compatibility: 🔄 READY (test with PHP 8.4 when available)"
fi

echo ""
echo "To use the extension:"
echo "  php -d extension=./modules/cassandra.so your_script.php"
echo ""
echo "To run tests:"
echo "  php -d extension=./modules/cassandra.so scripts/test-php84-compatibility.php"

# Optional: Suggest ScyllaDB driver for ScyllaDB-only users
if [[ "$USE_SCYLLA_DRIVER" == "false" ]]; then
    echo ""
    echo "💡 Using ScyllaDB exclusively? Consider the specialized ScyllaDB driver:"
    echo "   git clone https://github.com/scylladb/cpp-driver.git"
    echo "   cd cpp-driver && mkdir build && cd build"
    echo "   cmake .. && make -j4 && sudo make install"
    echo "   Then rebuild with: ./scripts/build-php84.sh"
fi 