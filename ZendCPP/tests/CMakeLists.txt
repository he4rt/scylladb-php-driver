cmake_minimum_required(VERSION 3.16)

# Only set project if this is the top-level CMakeLists
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    project(ZendCPPTests CXX)

    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)

    # Add cmake modules path when building standalone
    list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../../cmake")

    # Find PHP when building standalone
    find_package(PHP REQUIRED)

    if (NOT PHP_FOUND)
        message(FATAL_ERROR "PHP not found. Please install PHP development files.")
    endif ()
else ()
    # When included from root CMake, PHP should already be found
    if (NOT PHP_FOUND)
        message(FATAL_ERROR "PHP not found. Parent CMakeLists should have called find_package(PHP)")
    endif ()
endif ()

# Ensure C++ standard is set (when included from root)
if (NOT CMAKE_CXX_STANDARD)
    set(CMAKE_CXX_STANDARD 23)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif ()

# Include directories
include_directories(
        ${PHP_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/../..  # For ZendCPP/ prefix includes
        ${CMAKE_CURRENT_SOURCE_DIR}/..     # For direct includes
        ${CMAKE_CURRENT_SOURCE_DIR}        # For local includes
)

# Compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-O2")

# Optional: AddressSanitizer
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if (ENABLE_ASAN)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fsanitize=address -fno-omit-frame-pointer")
    set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -fsanitize=address")
endif ()

# Test framework sources
set(TEST_FRAMEWORK_SOURCES
        framework/TestRunner.cpp
)

# Test case sources
set(TEST_CASE_SOURCES
        cases/HashTableTest.cpp
        cases/ArrayTest.cpp
        cases/StringBuilderTest.cpp
        cases/ExceptionTest.cpp
)

# StringBuilder dependency
set(STRINGBUILDER_SOURCES
        ../String/Builder.cpp
)

# Main test extension
add_library(zendcpp_test MODULE
        test_main.cpp
        ${TEST_FRAMEWORK_SOURCES}
        ${STRINGBUILDER_SOURCES}
)

# Set output name
set_target_properties(zendcpp_test PROPERTIES
        PREFIX ""
        OUTPUT_NAME "zendcpp_test"
        SUFFIX ".so"
)

# macOS-specific linker flags to allow undefined symbols (resolved by PHP at runtime)
if(APPLE)
    target_link_options(zendcpp_test PRIVATE -undefined dynamic_lookup)
endif()

# Custom target to run tests
add_custom_target(run_tests
        COMMAND ${PHP_EXECUTABLE} -d extension=$<TARGET_FILE:zendcpp_test> ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.php
        DEPENDS zendcpp_test
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Running ZendCPP tests..."
)

# Custom target for Valgrind
find_program(VALGRIND_EXECUTABLE valgrind)
if (VALGRIND_EXECUTABLE)
    add_custom_target(run_tests_valgrind
            COMMAND ${VALGRIND_EXECUTABLE}
            --leak-check=full
            --show-leak-kinds=definite
            --suppressions=${CMAKE_CURRENT_SOURCE_DIR}/valgrind.supp
            --error-exitcode=1
            ${PHP_EXECUTABLE} -d extension=$<TARGET_FILE:zendcpp_test> ${CMAKE_CURRENT_SOURCE_DIR}/run_tests.php
            DEPENDS zendcpp_test
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            COMMENT "Running ZendCPP tests with Valgrind..."
    )
endif ()

# IDE support: Create a pseudo-target that includes all sources for code completion
add_library(zendcpp_test_ide EXCLUDE_FROM_ALL
        test_main.cpp
        ${TEST_FRAMEWORK_SOURCES}
        ${TEST_CASE_SOURCES}
        ${STRINGBUILDER_SOURCES}
)

target_include_directories(zendcpp_test_ide PRIVATE
        ${PHP_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/..
        ${CMAKE_CURRENT_SOURCE_DIR}
)

# Install target (optional)
install(TARGETS zendcpp_test
        LIBRARY DESTINATION ${PHP_EXTENSION_DIR}
)

# Print configuration
message(STATUS "")
message(STATUS "ZendCPP Test Configuration:")
message(STATUS "  PHP Executable: ${PHP_EXECUTABLE}")
message(STATUS "  PHP Version: ${PHP_VERSION}")
message(STATUS "  PHP Include: ${PHP_INCLUDE_DIRS}")
message(STATUS "  Build Type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  C++ Standard: ${CMAKE_CXX_STANDARD}")
if (ENABLE_ASAN)
    message(STATUS "  AddressSanitizer: Enabled")
endif ()
message(STATUS "")
