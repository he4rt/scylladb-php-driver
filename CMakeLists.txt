cmake_minimum_required(VERSION 3.30)

project(php-ext-scylladb
        VERSION     1.4.0
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

set(PHP_SCYLLADB_DRIVER_VERSION "${PROJECT_VERSION}-dev")

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
option(LINK_LIBUV_STATIC  "Link libuv statically"                         OFF)
option(BUILD_LIBUV_FROM_SRC "Fetch and build libuv via CPM instead of system" OFF)
option(PHP_SCYLLADB_STATIC  "Link the C++ driver statically"                OFF)

# Backend selection — see cmake/FindCPPDriver.cmake. USE_LIBCASSANDRA is kept
# for backwards compatibility and is consumed there.
option(USE_LIBCASSANDRA   "(deprecated) Alias: ON → PHP_SCYLLADB_BACKEND=cassandra" OFF)

# ── Dependencies ──────────────────────────────────────────────────────────────
find_package(PHPConfig REQUIRED)
find_package(PHP      REQUIRED)

# Must come after find_package(PHP) — reads PHP_PREFIX, PHP_EXTENSION_DIR etc.
include(cmake/PhpExtensionHelpers.cmake)

# Build-time arginfo generation: .stub.php → _arginfo.h via vendored
# tools/gen_stub/gen_stub.php using the PHP that's being built against.
include(cmake/GenStubs.cmake)

if (ENABLE_SANITIZERS)
    set(_sanitizers_cmake_dir "${PROJECT_SOURCE_DIR}/third-party/sanitizers-cmake/cmake")
    if (NOT EXISTS "${_sanitizers_cmake_dir}/FindSanitizers.cmake")
        # Submodule not initialized — try to init it automatically when we're
        # inside a git checkout; otherwise tell the user exactly what to run.
        find_package(Git QUIET)
        if (Git_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.gitmodules")
            message(STATUS "Initializing third-party/sanitizers-cmake submodule")
            execute_process(
                COMMAND "${GIT_EXECUTABLE}" submodule update --init --recursive
                        third-party/sanitizers-cmake
                WORKING_DIRECTORY "${PROJECT_SOURCE_DIR}"
                RESULT_VARIABLE _sm_rc
            )
        else ()
            set(_sm_rc 1)
        endif ()
        if (NOT EXISTS "${_sanitizers_cmake_dir}/FindSanitizers.cmake")
            message(FATAL_ERROR
                "ENABLE_SANITIZERS=ON but third-party/sanitizers-cmake is missing.\n"
                "Run: git submodule update --init --recursive\n"
                "Or disable with: -DENABLE_SANITIZERS=OFF"
            )
        endif ()
        unset(_sm_rc)
    endif ()
    unset(_sanitizers_cmake_dir)
    find_package(Sanitizers REQUIRED)
endif ()

find_package(Libuv    REQUIRED)
find_package(LibGMP   REQUIRED)
find_package(CPPDriver REQUIRED)

# ── Sub-libraries ─────────────────────────────────────────────────────────────
add_subdirectory(src/Registry)
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
        PRIVATE src/php_scylladb.c
)

target_link_libraries(ext_scylladb PRIVATE
        Libuv::Libuv
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
