cmake_minimum_required(VERSION 3.30)

project(php-ext-scylladb
        VERSION     1.4.0
        DESCRIPTION "ScyllaDB/Cassandra PHP extension"
        HOMEPAGE_URL "https://github.com/he4rt/scylladb-php-driver"
        LANGUAGES   C CXX
)

add_library(ext_scylladb SHARED)

if (NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Build type" FORCE)
endif ()

# All targets in this project need PIC (static libs linked into a shared ext)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_MODULE_PATH
        "${PROJECT_SOURCE_DIR}/cmake"
        "${PROJECT_SOURCE_DIR}/third-party/sanitizers-cmake/cmake"
)

include(cmake/CPM.cmake)
include(cmake/TargetOptimizations.cmake)

set(PHP_SCYLLADB_DRIVER_VERSION "${PROJECT_VERSION}-dev")

# ── Library options ───────────────────────────────────────────────────────────
option(ENABLE_SANITIZERS "Enable AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
option(ENABLE_AVX        "Enable -mavx on x86_64" OFF)
option(ENABLE_AVX2       "Enable -mavx2 on x86_64" OFF)
option(ENABLE_LTO        "Enable link-time optimisation" OFF)
set(CPU_TYPE "x86-64-v2" CACHE STRING
        "x86-64 micro-architecture: x86-64 | x86-64-v2 | x86-64-v3 | x86-64-v4 | native")

# ── PHP options ───────────────────────────────────────────────────────────────
set(CUSTOM_PHP_CONFIG "" CACHE STRING
        "Absolute path to a specific php-config binary (overrides auto-detection)")
set(PHP_VERSION_FOR_PHP_CONFIG "8.4" CACHE STRING
        "PHP minor version used to locate php-config under php/<version>-{debug,release}-{nts,zts}/")
option(PHP_DEBUG       "Target a debug PHP build"       ON)
option(PHP_THREAD_SAFE "Target a ZTS (thread-safe) PHP build" OFF)

# ── Dependency options ────────────────────────────────────────────────────────
option(LINK_LIBUV_STATIC  "Link libuv statically"                         OFF)
option(BUILD_LIBUV_FROM_SRC "Fetch and build libuv via CPM instead of system" OFF)
option(PHP_DRIVER_STATIC  "Link the C++ driver statically"                OFF)
option(USE_LIBCASSANDRA   "Use DataStax libcassandra instead of libscylla-cpp-driver" OFF)

# ── Dependencies ──────────────────────────────────────────────────────────────
find_package(PHPConfig REQUIRED)
find_package(PHP      REQUIRED)

# Must come after find_package(PHP) — reads PHP_PREFIX, PHP_EXTENSION_DIR etc.
include(cmake/PhpExtensionHelpers.cmake)

if (ENABLE_SANITIZERS)
    find_package(Sanitizers REQUIRED)
endif ()

find_package(Libuv    REQUIRED)
find_package(LibGMP   REQUIRED)
find_package(CPPDriver REQUIRED)

# ── Sub-libraries ─────────────────────────────────────────────────────────────
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

# ── Main target ───────────────────────────────────────────────────────────────
file(GLOB_RECURSE HEADERS_LIST CONFIGURE_DEPENDS include/*.h)

scylladb_php_library(ext_scylladb)

target_sources(ext_scylladb
        PUBLIC  FILE_SET headers TYPE HEADERS FILES "${HEADERS_LIST}"
        PRIVATE src/php_driver.cpp
)

target_link_libraries(ext_scylladb PRIVATE
        Zend
        Libuv::Libuv
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

configure_file(version.h.in version.h USE_SOURCE_PERMISSIONS NEWLINE_STYLE UNIX)

target_compile_definitions(ext_scylladb PRIVATE COMPILE_DL_CASSANDRA)

set_target_properties(ext_scylladb PROPERTIES
        PREFIX      ""
        OUTPUT_NAME "cassandra"
)

# ── Install ───────────────────────────────────────────────────────────────────
# php_extension_install() is defined in cmake/PhpExtensionHelpers.cmake.
# It installs cassandra.so and cassandra.ini as separate components and
# sets CMAKE_INSTALL_PREFIX to PHP_PREFIX so DESTDIR works correctly.
php_extension_install(ext_scylladb)
