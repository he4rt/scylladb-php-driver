#!/bin/bash
# Quick start script to build and test ZendCPP

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "================================================"
echo "ZendCPP Test Framework"
echo "================================================"
echo ""

# Parse command
case "${1:-help}" in
    build)
        echo "Building test extension..."
        cd tests

        phpize --clean 2>/dev/null || true
        phpize
        ./configure --enable-zendcpp_test
        make clean
        make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

        cd "$SCRIPT_DIR"
        echo ""
        echo "✓ Test extension built successfully!"
        echo "Extension: tests/modules/zendcpp_test.so"
        ;;

    test)
        echo "Running all tests..."
        cd tests

        if [ ! -f "modules/zendcpp_test.so" ]; then
            echo "⚠️  Extension not built. Run: $0 build"
            exit 1
        fi

        php -d extension=modules/zendcpp_test.so run_tests.php
        cd "$SCRIPT_DIR"
        ;;

    quick)
        echo "Quick test (build & run all tests)..."
        echo ""

        cd tests
        phpize --clean 2>/dev/null || true
        phpize
        ./configure --enable-zendcpp_test
        make clean
        make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

        echo ""
        echo "Running tests..."
        php -d extension=modules/zendcpp_test.so run_tests.php

        cd "$SCRIPT_DIR"
        echo ""
        echo "✓ Quick test completed!"
        ;;

    valgrind)
        echo "Running tests with Valgrind..."
        cd tests

        if [ ! -f "modules/zendcpp_test.so" ]; then
            echo "⚠️  Extension not built. Run: $0 build"
            exit 1
        fi

        valgrind --leak-check=full \
                 --show-leak-kinds=definite \
                 --suppressions=valgrind.supp \
                 --error-exitcode=1 \
                 php -d extension=modules/zendcpp_test.so run_tests.php

        cd "$SCRIPT_DIR"
        ;;

    asan)
        echo "Building with AddressSanitizer..."
        cd tests

        phpize --clean 2>/dev/null || true
        phpize
        CXXFLAGS="-fsanitize=address -g -O0" ./configure --enable-zendcpp_test
        make clean
        make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

        echo ""
        echo "Running tests with ASan..."
        ASAN_OPTIONS=detect_leaks=1 \
        php -d extension=modules/zendcpp_test.so run_tests.php

        cd "$SCRIPT_DIR"
        ;;

    clean)
        echo "Cleaning build artifacts..."
        cd tests
        make clean 2>/dev/null || true
        phpize --clean 2>/dev/null || true
        cd "$SCRIPT_DIR"
        echo "✓ Cleaned"
        ;;

    help|*)
        echo "Usage: $0 {build|test|quick|valgrind|asan|clean|help}"
        echo ""
        echo "Commands:"
        echo "  build    - Build the test extension"
        echo "  test     - Run all tests (requires build first)"
        echo "  quick    - Quick test: build & run all tests"
        echo "  valgrind - Run tests with Valgrind leak detection"
        echo "  asan     - Build with AddressSanitizer and run tests"
        echo "  clean    - Clean all build artifacts"
        echo "  help     - Show this message"
        echo ""
        echo "Build Methods:"
        echo "  • phpize (this script) - Traditional PHP extension build"
        echo "  • CMake - Modern build with IDE support"
        echo "    cd tests && ./cmake_build.sh --run"
        echo "    See tests/CMAKE.md for details"
        echo ""
        echo "Test Framework:"
        echo "  • Tests are in separate files: tests/cases/*.cpp"
        echo "  • All compile into ONE extension"
        echo "  • Auto-register using ZENDCPP_TEST macro"
        echo "  • Easy to add new tests"
        echo ""
        echo "Adding Tests:"
        echo "  1. Create tests/cases/MyTest.cpp"
        echo "  2. Use ZENDCPP_TEST(Category, name) { ... }"
        echo "  3. Include in tests/test_main.cpp"
        echo "  4. Rebuild and run"
        echo ""
        echo "Examples:"
        echo "  $0 quick      # Quick verification"
        echo "  $0 valgrind   # Check for memory leaks"
        echo "  $0 asan       # Run with AddressSanitizer"
        echo ""
        echo "For IDE support and code completion, use CMake:"
        echo "  cd tests && ./cmake_build.sh"
        ;;
esac
