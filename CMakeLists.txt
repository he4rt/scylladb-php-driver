cmake_minimum_required(VERSION 3.24)

if(POLICY CMP0074)
    cmake_policy(SET CMP0074 NEW)
endif()

if(POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

if(POLICY CMP0079)
    cmake_policy(SET CMP0079 NEW)
endif()

if(POLICY CMP0108)
    cmake_policy(SET CMP0108 NEW)
endif()

if(POLICY CMP0109)
    cmake_policy(SET CMP0109 NEW)
endif()

if(POLICY CMP0128)
    cmake_policy(SET CMP0128 NEW)
endif()

project(php-ext-scylladb LANGUAGES C CXX)
add_library(ext_scylladb SHARED)

# if build type is not set, default to debug
if(NOT CMAKE_BUILD_TYPE OR CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "Debug")
endif()

include(CheckSymbolExists)
include(CheckIncludeFile)
include(CMakePackageConfigHelpers)
include(cmake/CPM.cmake)
include(cmake/TargetOptimizations.cmake)
include(cmake/ScyllaDBPHPLibrary.cmake)

set(PHP_SCYLLADB_VERSION "1.4.0-dev")

set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake" "${PROJECT_SOURCE_DIR}/third-party/sanitizers-cmake/cmake")

# Library Options
option(PHP_SCYLLADB_ENABLE_SANITIZERS "Enable sanitizers" OFF)
option(PHP_SCYLLADB_ENABLE_COVERAGE "Enable coverage" OFF)
option(PHP_SCYLLADB_OPTIMISE_FOR_CURRENT_MACHINE "Optimise for current machine" OFF)

# PHP Options
set(CUSTOM_PHP_CONFIG "" CACHE STRING "Custom PHP config path")
set(PHP_VERSION_FOR_PHP_CONFIG "8.3" CACHE STRING "PHP version")

option(PHP_DEBUG_FOR_PHP_CONFIG "Debug or Release" ON)
option(PHP_THREAD_SAFE_FOR_PHP_CONFIG "ZTS(zts) or NTS(nts)" OFF)

if(CMAKE_HOST_APPLE)
    option(PHP_SCYLLADB_LIBUV_STATIC "Statically link libuv" ON)
else()
    option(PHP_SCYLLADB_LIBUV_STATIC "Statically link libuv" OFF)
endif()

option(PHP_SCYLLADB_LIBUV_FROM_SRC "Build LibUV from Source" ON)

if(CMAKE_HOST_APPLE)
    option(PHP_SCYLLADB_LIBSCYLLADB_STATIC "Statically link LibScyllaDB" ON)
else()
    option(PHP_SCYLLADB_LIBSCYLLADB_STATIC "Statically link LibScyllaDB" OFF)
endif()

option(PHP_SCYLLADB_USE_LIBCASSANDRA "Use DataStax LibCassandra instead of LibScyllaDB" OFF)

if(CMAKE_HOST_APPLE)
    option(PHP_SCYLLADB_LIBCASSANDRA_STATIC "Statically link LibCassandra" ON)
else()
    option(PHP_SCYLLADB_LIBCASSANDRA_STATIC "Statically link LibCassandra" OFF)
endif()

find_package(PHPConfig REQUIRED)
find_package(PHP REQUIRED)
find_package(Sanitizers REQUIRED)
find_package(Libuv REQUIRED)
find_package(LibGMP REQUIRED)

if(PHP_SCYLLADB_USE_LIBCASSANDRA)
    find_package(CassandraDriver REQUIRED)
else()
    find_package(ScyllaDBDriver REQUIRED)
endif()

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

scylladb_php_library(ext_scylladb ${PHP_SCYLLADB_ENABLE_SANITIZERS} ${PHP_SCYLLADB_OPTIMISE_FOR_CURRENT_MACHINE} OFF)

target_sources(ext_scylladb PUBLIC FILE_SET headers TYPE HEADERS FILES "${HEADERS_LIST}" "${PROJECT_BINARY_DIR}/config.h")
target_sources(ext_scylladb PRIVATE php_driver.cpp)

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

check_include_file("dlfcn.h" HAVE_DLFCN_H)
check_include_file("stdio.h" HAVE_STDIO_H)
check_include_file("stdint.h" HAVE_STDINT_H)
check_include_file("stdlib.h" HAVE_STDLIB_H)
check_include_file("inttypes.h" HAVE_INTTYPES_H)
check_include_file("sys/stat.h" HAVE_SYS_STAT_H)
check_include_file("sys/types.h" HAVE_SYS_TYPES_H)
check_include_file("string.h" HAVE_STRING_H)
check_include_file("unistd.h" HAVE_UNISTD_H)

set(HAVE_STRINGS_H 0)

configure_file(
    config.in
    config.h
    USE_SOURCE_PERMISSIONS
    NEWLINE_STYLE UNIX
)

configure_file(
    version.h.in
    version.h
    USE_SOURCE_PERMISSIONS
    NEWLINE_STYLE UNIX
)

target_compile_definitions(ext_scylladb PRIVATE -DCOMPILE_DL_CASSANDRA -DHAVE_CONFIG_H)
set_target_properties(ext_scylladb PROPERTIES POSITION_INDEPENDENT_CODE ON)
set_target_properties(ext_scylladb PROPERTIES PREFIX "")
set_target_properties(ext_scylladb PROPERTIES OUTPUT_NAME "cassandra")

install(TARGETS ext_scylladb DESTINATION "${PHP_EXTENSION_DIR}")
install(TARGETS ext_scylladb FILE_SET headers DESTINATION "${PHP_INCLUDES_DIR}/ext/cassandra")
