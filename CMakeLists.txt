cmake_minimum_required(VERSION 3.30)

if (POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif ()

if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif ()

if (POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif ()

if (POLICY CMP0108)
    cmake_policy(SET CMP0108 NEW)
endif ()

if (POLICY CMP0109)
    cmake_policy(SET CMP0109 NEW)
endif ()

if (POLICY CMP0128)
    cmake_policy(SET CMP0128 NEW)
endif ()

project(php-ext-scylladb LANGUAGES C CXX)
add_library(ext_scylladb SHARED)

# if build type is not set, default to debug
if (NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug")
endif ()

include(CheckSymbolExists)
include(CheckIncludeFile)
include(CMakePackageConfigHelpers)
include(cmake/CPM.cmake)
include(cmake/TargetOptimizations.cmake)

set(PHP_SCYLLADB_DRIVER_VERSION "1.4.0-dev")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake" "${PROJECT_SOURCE_DIR}/third-party/sanitizers-cmake/cmake")

# Library Options
option(ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(ENABLE_AVX "Enable AVX" OFF)
option(ENABLE_AVX2 "Enable AVX2" OFF)
option(ENABLE_LTO "Enable LTO" OFF)
set(CPU_TYPE "x86-64-v2" CACHE STRING "For x86_64 = x86-64|x86-64-v2|x86-64-v3|x86-64-v4|native")

# PHP Options
set(CUSTOM_PHP_CONFIG "" CACHE STRING "Custom PHP config path")
set(PHP_VERSION_FOR_PHP_CONFIG "8.4" CACHE STRING "PHP version")
option(PHP_DEBUG "Debug or Release" ON)
option(PHP_THREAD_SAFE "ZTS(zts) or NTS(nts)" OFF)

option(LINK_LIBUV_STATIC "Statically link libuv" OFF)
option(BUILD_LIBUV_FROM_SRC "Build LibUV from Source" OFF)

option(PHP_DRIVER_STATIC "Statically link PHP Driver" OFF)
option(USE_LIBCASSANDRA "Use DataStax LibCassandra instead of LibScyllaDB" OFF)

find_package(PHPConfig REQUIRED)
find_package(PHP REQUIRED)
find_package(Sanitizers REQUIRED)
find_package(Libuv REQUIRED)
find_package(LibGMP REQUIRED)
find_package(CPPDriver REQUIRED)

add_subdirectory(ZendCPP)
add_subdirectory(util)
add_subdirectory(src)
add_subdirectory(src/Cluster)
add_subdirectory(src/DateTime)
add_subdirectory(src/Database)
add_subdirectory(src/Numbers)
add_subdirectory(src/Exception)
add_subdirectory(src/RetryPolicy)
add_subdirectory(src/SSLOptions)
add_subdirectory(src/TimestampGenerator)
add_subdirectory(src/Type)

file(GLOB_RECURSE HEADERS_LIST FOLLOW_SYMLINKS include/*.h)

scylladb_php_library(ext_scylladb "${ENABLE_SANITIZERS}" "${CPU_TYPE}" "${ENABLE_LTO}")

target_sources(ext_scylladb PUBLIC FILE_SET headers TYPE HEADERS FILES "${HEADERS_LIST}")
target_sources(
        ext_scylladb
        PRIVATE
        src/php_driver.cpp
)
target_link_libraries(
        ext_scylladb PRIVATE
        Zend
        ext_scylladb::src
        ext_scylladb::utils
        ext_scylladb::type
        ext_scylladb::datetime
        ext_scylladb::database
        ext_scylladb::numbers
        ext_scylladb::ssl_options
        ext_scylladb::exceptions
        ext_scylladb::cluster::builder
        ext_scylladb::retry_policy
        ext_scylladb::timestamp_generator
)

configure_file(
        version.h.in
        version.h
        USE_SOURCE_PERMISSIONS
        NEWLINE_STYLE UNIX
)

target_compile_definitions(ext_scylladb PRIVATE -DCOMPILE_DL_CASSANDRA)
set_target_properties(ext_scylladb PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(ext_scylladb PROPERTIES PREFIX "")
set_target_properties(ext_scylladb PROPERTIES OUTPUT_NAME "cassandra")
