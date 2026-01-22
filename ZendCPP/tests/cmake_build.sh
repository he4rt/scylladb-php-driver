#!/bin/bash
# CMake build script for ZendCPP tests
# Can be run standalone or as part of root project build

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="${SCRIPT_DIR}/../.."
BUILD_DIR="${SCRIPT_DIR}/build"
USE_ROOT_BUILD=false

usage() {
    echo "Usage: $0 [options]"
    echo ""
    echo "Options:"
    echo "  --clean         Clean build directory"
    echo "  --debug         Debug build (default)"
    echo "  --release       Release build"
    echo "  --asan          Enable AddressSanitizer"
    echo "  --run           Run tests after build"
    echo "  --valgrind      Run tests with Valgrind"
    echo "  --root          Build from project root (recommended)"
    echo "  --help          Show this help"
    echo ""
    echo "Examples:"
    echo "  $0 --root --run                # Build from root and run tests"
    echo "  $0 --release --run              # Build standalone in release and run tests"
    echo "  $0 --root --asan --run          # Build from root with ASan and run tests"
    echo "  $0 --clean                      # Clean build directory"
}

# Default options
BUILD_TYPE="Debug"
CLEAN=false
RUN_TESTS=false
RUN_VALGRIND=false
ENABLE_ASAN=OFF

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --clean)
            CLEAN=true
            shift
            ;;
        --debug)
            BUILD_TYPE="Debug"
            shift
            ;;
        --release)
            BUILD_TYPE="Release"
            shift
            ;;
        --asan)
            ENABLE_ASAN=ON
            shift
            ;;
        --run)
            RUN_TESTS=true
            shift
            ;;
        --valgrind)
            RUN_VALGRIND=true
            shift
            ;;
        --root)
            USE_ROOT_BUILD=true
            BUILD_DIR="${PROJECT_ROOT}/build"
            shift
            ;;
        --help)
            usage
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Clean if requested
if [ "$CLEAN" = true ]; then
    echo "Cleaning build directory..."
    if [ "$USE_ROOT_BUILD" = true ]; then
        rm -rf "$BUILD_DIR"
    else
        rm -rf "${SCRIPT_DIR}/build"
    fi
    echo "✓ Clean complete"
    exit 0
fi

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
echo "Configuring CMake..."
if [ "$USE_ROOT_BUILD" = true ]; then
    # Build from project root with ZendCPP tests enabled
    cmake "$PROJECT_ROOT" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DENABLE_ASAN="$ENABLE_ASAN" \
        -DBUILD_ZENDCPP_TESTS=ON \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
else
    # Build standalone (just ZendCPP tests)
    cmake "${SCRIPT_DIR}" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DENABLE_ASAN="$ENABLE_ASAN" \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

# Build
echo ""
echo "Building..."
if [ "$USE_ROOT_BUILD" = true ]; then
    # Build only the ZendCPP test target
    cmake --build . --target zendcpp_test -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
else
    cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
fi

echo ""
echo "✓ Build complete!"

# Find the extension
if [ "$USE_ROOT_BUILD" = true ]; then
    EXTENSION_PATH="$BUILD_DIR/ZendCPP/tests/zendcpp_test.so"
else
    EXTENSION_PATH="$BUILD_DIR/zendcpp_test.so"
fi

echo "Extension: $EXTENSION_PATH"

# Copy compile_commands.json for IDE support
if [ -f "compile_commands.json" ]; then
    cp compile_commands.json "$SCRIPT_DIR/"
    echo "✓ Generated compile_commands.json for IDE support"
fi

# Run tests if requested
if [ "$RUN_TESTS" = true ]; then
    echo ""
    echo "Running tests..."

    # Determine PHP executable to use
    PHP_BIN="${PHP_EXECUTABLE:-php}"

    # Check if PHP is available
    if ! command -v "$PHP_BIN" &> /dev/null; then
        echo "❌ PHP not found: $PHP_BIN"
        echo "Set PHP_EXECUTABLE environment variable to specify PHP binary"
        exit 1
    fi

    echo "Using PHP: $PHP_BIN ($($PHP_BIN --version | head -1))"

    # Check if extension exists
    if [ ! -f "$EXTENSION_PATH" ]; then
        echo "❌ Extension not found: $EXTENSION_PATH"
        exit 1
    fi

    # Run tests directly with PHP
    echo "Loading extension: $EXTENSION_PATH"
    "$PHP_BIN" -d extension="$EXTENSION_PATH" "$SCRIPT_DIR/run_tests.php"
    TEST_EXIT_CODE=$?

    if [ $TEST_EXIT_CODE -eq 0 ]; then
        echo "✅ Tests completed successfully!"
    else
        echo "❌ Tests failed with exit code: $TEST_EXIT_CODE"
        exit $TEST_EXIT_CODE
    fi
fi

# Run with Valgrind if requested
if [ "$RUN_VALGRIND" = true ]; then
    echo ""
    echo "Running tests with Valgrind..."

    PHP_BIN="${PHP_EXECUTABLE:-php}"

    if ! command -v valgrind &> /dev/null; then
        echo "⚠️  Valgrind not found, skipping memory check"
        exit 1
    fi

    if [ ! -f "$EXTENSION_PATH" ]; then
        echo "❌ Extension not found: $EXTENSION_PATH"
        exit 1
    fi

    echo "Using PHP: $PHP_BIN"
    echo "Using Valgrind: $(valgrind --version | head -1)"

    valgrind \
        --leak-check=full \
        --show-leak-kinds=definite \
        --suppressions="$SCRIPT_DIR/valgrind.supp" \
        --error-exitcode=1 \
        "$PHP_BIN" -d extension="$EXTENSION_PATH" "$SCRIPT_DIR/run_tests.php"

    VALGRIND_EXIT_CODE=$?

    if [ $VALGRIND_EXIT_CODE -eq 0 ]; then
        echo "✅ Valgrind check passed!"
    else
        echo "❌ Valgrind detected memory issues (exit code: $VALGRIND_EXIT_CODE)"
        exit $VALGRIND_EXIT_CODE
    fi
fi

echo ""
echo "To run tests manually:"
echo "  PHP_EXECUTABLE=\$(which php) # Or custom path"
echo "  \$PHP_EXECUTABLE -d extension=$EXTENSION_PATH $SCRIPT_DIR/run_tests.php"
