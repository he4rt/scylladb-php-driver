cmake_minimum_required(VERSION 3.30)

project(php-ext-scylladb
        VERSION     1.5.1
        DESCRIPTION "ScyllaDB/Cassandra PHP extension"
        HOMEPAGE_URL "https://github.com/he4rt/scylladb-php-driver"
        LANGUAGES   C
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

# Release builds pass the git tag in; every other build is a snapshot of the
# next release, so it carries the -dev suffix. .github/workflows/release.yml
# refuses to build when the tag and PROJECT_VERSION disagree.
set(PHP_SCYLLADB_DRIVER_VERSION "" CACHE STRING
        "Extension version reported by phpversion('cassandra'). Empty = ${PROJECT_VERSION}-dev")

if (NOT PHP_SCYLLADB_DRIVER_VERSION)
    set(PHP_SCYLLADB_DRIVER_VERSION "${PROJECT_VERSION}-dev")
endif ()

message(STATUS "Extension version: ${PHP_SCYLLADB_DRIVER_VERSION}")

# ── macOS / Homebrew prefix detection ────────────────────────────────────────
# Homebrew on Apple Silicon installs to /opt/homebrew; on Intel to /usr/local.
# Adding the prefix to CMAKE_PREFIX_PATH lets find_package / pkg-config locate
# libraries without requiring the user to set PKG_CONFIG_PATH manually.
#
# Several formulae we depend on are keg-only (openssl@3, libuv, libssh2, gmp)
# — their headers/pkgconfig live under their own prefix, not the global one —
# so we probe each individually and add them too.
if (APPLE)
    function(_scylladb_add_brew_prefix formula)
        execute_process(
                COMMAND brew --prefix ${formula}
                OUTPUT_VARIABLE _p
                OUTPUT_STRIP_TRAILING_WHITESPACE
                ERROR_QUIET
                RESULT_VARIABLE _rc
        )
        if (_rc EQUAL 0 AND _p AND IS_DIRECTORY "${_p}")
            list(PREPEND CMAKE_PREFIX_PATH "${_p}")
            set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)
            set(ENV{PKG_CONFIG_PATH} "${_p}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
            message(STATUS "Homebrew prefix [${formula}]: ${_p}")
        endif ()
    endfunction()

    _scylladb_add_brew_prefix("")          # global brew prefix
    _scylladb_add_brew_prefix(openssl@3)
    _scylladb_add_brew_prefix(libuv)
    _scylladb_add_brew_prefix(libssh2)
    _scylladb_add_brew_prefix(gmp)
endif ()

# ── C++ driver prefix selection ──────────────────────────────────────────────
# When the cpp-driver is installed under a non-standard location (e.g.
# /usr/local/scylladb or /usr/local/cassandra from scripts/compile-cpp-driver.sh
# with the per-driver layout), prepend its pkgconfig dir so FindCPPDriver picks
# the intended one — this is also how you switch between scylladb / cassandra
# when both are installed side by side.
set(CPP_DRIVER_PREFIX "" CACHE PATH
        "Install prefix where the chosen cpp-driver was installed \
(e.g. /usr/local/scylladb or /usr/local/cassandra). \
Its lib/pkgconfig dir is prepended to PKG_CONFIG_PATH.")
if (CPP_DRIVER_PREFIX)
    if (NOT IS_DIRECTORY "${CPP_DRIVER_PREFIX}")
        message(WARNING "CPP_DRIVER_PREFIX='${CPP_DRIVER_PREFIX}' does not exist")
    endif ()
    list(PREPEND CMAKE_PREFIX_PATH "${CPP_DRIVER_PREFIX}")
    set(ENV{PKG_CONFIG_PATH} "${CPP_DRIVER_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
    message(STATUS "CPP driver prefix: ${CPP_DRIVER_PREFIX}")
endif ()

# ── Library options ───────────────────────────────────────────────────────────
option(ENABLE_SANITIZERS "Enable AddressSanitizer + UndefinedBehaviorSanitizer" OFF)
option(ENABLE_AVX        "Enable -mavx on x86_64" OFF)
option(ENABLE_AVX2       "Enable -mavx2 on x86_64" OFF)
option(ENABLE_LTO        "Enable link-time optimisation" OFF)
option(PHP_SCYLLADB_WERROR          "Turn compiler warnings into errors" OFF)
option(PHP_SCYLLADB_HARDENING       "Build with stack protection and _FORTIFY_SOURCE" ON)

# ARM64 (Apple Silicon / AArch64) has no x86 micro-architecture levels;
# use "native" so the compiler optimises for the host without invalid flags.
if (CMAKE_SYSTEM_PROCESSOR MATCHES "arm64|aarch64")
    set(_default_cpu_type "native")
else ()
    set(_default_cpu_type "x86-64-v2")
endif ()
set(CPU_TYPE "${_default_cpu_type}" CACHE STRING
        "x86-64 micro-architecture: x86-64 | x86-64-v2 | x86-64-v3 | x86-64-v4 | native")
unset(_default_cpu_type)

# ── PHP options ───────────────────────────────────────────────────────────────
set(CUSTOM_PHP_CONFIG "" CACHE STRING
        "Absolute path to a specific php-config binary (overrides auto-detection)")
set(PHP_VERSION_FOR_PHP_CONFIG "8.4" CACHE STRING
        "PHP minor version used to locate php-config under php/<version>-{debug,release}-{nts,zts}/")
option(PHP_DEBUG       "Target a debug PHP build"       ON)
option(PHP_THREAD_SAFE "Target a ZTS (thread-safe) PHP build" OFF)

# ── Dependency options ────────────────────────────────────────────────────────
option(PHP_SCYLLADB_STATIC  "Link the C++ driver statically"                OFF)

# ── Native Swoole / OpenSwoole coroutine support (opt-in) ─────────────────────
# When enabled, one C++ shim (src/Async/SwooleBridge.cc) is compiled against the
# (open)swoole source headers so Future::get() suspends the current coroutine
# instead of blocking. OFF by default keeps the build pure C. Requires the
# matching source tree via -DPHP_SCYLLADB_SWOOLE_SRC=/path/to/(open)swoole-src.
option(PHP_SCYLLADB_ENABLE_SWOOLE     "Build native Swoole coroutine support"     OFF)
option(PHP_SCYLLADB_ENABLE_OPENSWOOLE "Build native OpenSwoole coroutine support" OFF)
set(PHP_SCYLLADB_SWOOLE_SRC "" CACHE PATH
        "Path to the (open)swoole-src tree providing swoole.h / swoole_coroutine*.h")

# ── Io\Poll integration (PHP >= 8.6) ──────────────────────────────────────────
# PHP 8.6 added an epoll/kqueue polling API (RFC: poll_api) with an internal
# handle interface. When enabled, src/Async/Poll.c and PollHandle.c are compiled
# and Cassandra\Async\Poll / PollHandle expose the driver's completion
# descriptors to it natively.
#
# OFF by default until PHP 8.6 is released. The API is still moving in 8.6-dev —
# Io\Poll\Context::wait() changed shape after 8.6.0alpha3 — so a default-on build
# would track a moving target. Turn it on explicitly:
#
#   cmake --preset DebugPHP8.6NTS -DPHP_SCYLLADB_ENABLE_POLL_API=ON
#
# AUTO enables it whenever the PHP being built against provides main/php_poll.h;
# ON does the same but makes a missing header a hard error.
set(PHP_SCYLLADB_ENABLE_POLL_API "OFF" CACHE STRING
        "Io\\Poll integration: OFF (default), AUTO (on when PHP >= 8.6 provides php_poll.h), ON (require)")
set_property(CACHE PHP_SCYLLADB_ENABLE_POLL_API PROPERTY STRINGS AUTO ON OFF)

option(PHP_SCYLLADB_ENABLE_LEGACY_SCHEMA_META
        "Build Cassandra\\Function_, Aggregate and Index. The Rust based C/C++ driver leaves cass_function_meta_*, cass_aggregate_meta_* and cass_index_meta_* unimplemented, so Keyspace::function(), functions(), aggregate(), aggregates() and Table::index(), indexes() throw while this is OFF."
        OFF)

option(PHP_SCYLLADB_ENABLE_CUSTOM_TYPE
        "Build Cassandra\\Custom and Cassandra\\Type\\Custom. The Rust based C/C++ driver does not plan to implement the cass_*_bind_custom family."
        OFF)

if (PHP_SCYLLADB_ENABLE_LEGACY_SCHEMA_META)
    add_compile_definitions(PHP_SCYLLADB_HAVE_LEGACY_SCHEMA_META)
endif ()

if (PHP_SCYLLADB_ENABLE_CUSTOM_TYPE)
    add_compile_definitions(PHP_SCYLLADB_HAVE_CUSTOM_TYPE)
endif ()

# Backend selection — see cmake/FindCPPDriver.cmake. USE_LIBCASSANDRA is kept
# for backwards compatibility and is consumed there.
option(USE_LIBCASSANDRA   "(deprecated) Alias: ON → PHP_SCYLLADB_BACKEND=cassandra" OFF)

# The scylla-cpp backend builds the vendored driver in third-party/cpp-driver by
# default, so a checkout builds with no driver installed and local fixes to the
# driver ship with the extension. ON restores the pkg-config lookup against an
# installed driver. The cassandra and scylla-rust backends are always external.
option(PHP_SCYLLADB_USE_SYSTEM_DRIVER
        "Link the system C/C++ driver found through pkg-config instead of third-party/cpp-driver" OFF)

# ── Dependencies ──────────────────────────────────────────────────────────────
find_package(PHPConfig REQUIRED)
find_package(PHP      REQUIRED)

# ── Io\Poll capability probe ──────────────────────────────────────────────────
# Needs PHP_VERSION_NUM and PHP_INCLUDES from find_package(PHP). php-config
# --includes lists both <include>/php and <include>/php/main, so search the base
# name and the main/ suffix. Result is read by src/CMakeLists.txt.
set(PHP_SCYLLADB_HAVE_POLL_API OFF)
if (NOT PHP_SCYLLADB_ENABLE_POLL_API STREQUAL "OFF")
    find_file(PHP_SCYLLADB_PHP_POLL_HEADER
            NAMES php_poll.h
            PATHS ${PHP_INCLUDES}
            PATH_SUFFIXES main
            NO_DEFAULT_PATH
            DOC "Path to PHP's main/php_poll.h (PHP >= 8.6)"
    )

    if (PHP_VERSION_NUM GREATER_EQUAL 80600 AND PHP_SCYLLADB_PHP_POLL_HEADER)
        set(PHP_SCYLLADB_HAVE_POLL_API ON)
    elseif (PHP_SCYLLADB_ENABLE_POLL_API STREQUAL "ON")
        message(FATAL_ERROR
                "PHP_SCYLLADB_ENABLE_POLL_API=ON but the polling API is not available: "
                "PHP ${PHP_VERSION} (vernum ${PHP_VERSION_NUM}) and php_poll.h "
                "'${PHP_SCYLLADB_PHP_POLL_HEADER}'. It needs PHP >= 8.6. Use AUTO or OFF.")
    endif ()
endif ()

# Must come after find_package(PHP) — reads PHP_PREFIX, PHP_EXTENSION_DIR etc.
include(cmake/PhpExtensionHelpers.cmake)

# Build-time arginfo generation: .stub.php → _arginfo.h via vendored
# tools/gen_stub/gen_stub.php using the PHP that's being built against.
include(cmake/GenStubs.cmake)

if (ENABLE_SANITIZERS)
    # sanitizers-cmake is vendored under third-party/ (no submodule). The path
    # is added to CMAKE_MODULE_PATH at the top of this file.
    if (NOT EXISTS "${PROJECT_SOURCE_DIR}/third-party/sanitizers-cmake/cmake/FindSanitizers.cmake")
        message(FATAL_ERROR
            "ENABLE_SANITIZERS=ON but third-party/sanitizers-cmake is missing "
            "(expected a vendored copy of arsenm/sanitizers-cmake under "
            "third-party/sanitizers-cmake/). Re-clone the repo or "
            "disable with: -DENABLE_SANITIZERS=OFF")
    endif ()
    find_package(Sanitizers REQUIRED)
endif ()

find_package(LibGMP   REQUIRED)

include(DriverBackend)

# Both paths define CppDriver::Driver; nothing downstream knows which one ran.
if (PHP_SCYLLADB_BACKEND STREQUAL "scylla-cpp" AND NOT PHP_SCYLLADB_USE_SYSTEM_DRIVER)
    include(VendoredCppDriver)
else ()
    find_package(CPPDriver REQUIRED)
endif ()

# ── Sub-libraries ─────────────────────────────────────────────────────────────
add_subdirectory(src/Registry)
add_subdirectory(src)
add_subdirectory(src/Cluster)
add_subdirectory(src/DateTime)
add_subdirectory(src/Database)
add_subdirectory(src/Numbers)
add_subdirectory(src/Exception)
add_subdirectory(src/RetryPolicy)
add_subdirectory(src/ExecutionProfile)
add_subdirectory(src/SSLOptions)
add_subdirectory(src/TimestampGenerator)
add_subdirectory(src/Type)

# ── Main target ───────────────────────────────────────────────────────────────
file(GLOB_RECURSE HEADERS_LIST CONFIGURE_DEPENDS include/*.h)

scylladb_php_library(ext_scylladb)

target_sources(ext_scylladb
        PUBLIC  FILE_SET headers TYPE HEADERS FILES "${HEADERS_LIST}"
        PRIVATE src/php_scylladb.c
)

target_link_libraries(ext_scylladb PRIVATE
        # Self-registering class descriptors live in these static libs. Their
        # .o files have no externally-referenced symbols once the legacy
        # define_*() functions are gone — without WHOLE_ARCHIVE the linker
        # would drop them and the __attribute__((constructor)) registrations
        # would never run.
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,ext_scylladb::src>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,ext_scylladb::type>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,ext_scylladb::datetime>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,ext_scylladb::database>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,ext_scylladb::numbers>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,ext_scylladb::execution_profile>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,ext_scylladb::ssl_options>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,ext_scylladb::exceptions>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,ext_scylladb::cluster::builder>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,ext_scylladb::retry_policy>"
        "$<LINK_LIBRARY:WHOLE_ARCHIVE,ext_scylladb::timestamp_generator>"
        ext_scylladb::registry
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
