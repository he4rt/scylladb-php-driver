include_guard(GLOBAL)

# Builds the vendored ScyllaDB cpp-driver under third-party/cpp-driver and
# exposes it as CppDriver::Driver, the same target cmake/FindCPPDriver.cmake
# creates from a system install. See third-party/cpp-driver/UPSTREAM for the
# imported commit.

find_package(PkgConfig REQUIRED)

set(_cass_root "${PROJECT_SOURCE_DIR}/third-party/cpp-driver")

if (NOT EXISTS "${_cass_root}/CMakeLists.txt")
    message(FATAL_ERROR
        "PHP_SCYLLADB_USE_SYSTEM_DRIVER=OFF but the vendored driver is missing "
        "(expected third-party/cpp-driver/CMakeLists.txt). Re-clone the repo, or "
        "build against an installed driver with: -DPHP_SCYLLADB_USE_SYSTEM_DRIVER=ON")
endif ()

# Static only: the driver's object code goes straight into cassandra.so, so the
# extension has no runtime dependency on a libscylla-cpp-driver.so.
set(CASS_BUILD_STATIC ON  CACHE BOOL "" FORCE)
set(CASS_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(CASS_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CASS_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(CASS_BUILD_UNIT_TESTS OFF CACHE BOOL "" FORCE)
set(CASS_BUILD_INTEGRATION_TESTS OFF CACHE BOOL "" FORCE)

# We consume the driver from the build tree, so it must not contribute install
# rules or a pkg-config file to `cmake --install` of this project.
set(CASS_INSTALL_HEADER OFF CACHE BOOL "" FORCE)
set(CASS_INSTALL_PKG_CONFIG OFF CACHE BOOL "" FORCE)

set(CASS_USE_OPENSSL ON CACHE BOOL "" FORCE)
set(CASS_USE_ZLIB ON CACHE BOOL "" FORCE)

# The driver's FindLibuv/FindOpenSSL take a *_ROOT_DIR hint and do not read
# CMAKE_PREFIX_PATH, so keg-only Homebrew formulae stay invisible to them. Feed
# the prefixes the root CMakeLists already resolved.
if (NOT LIBUV_ROOT_DIR)
    pkg_check_modules(_vendored_libuv QUIET libuv)
    if (_vendored_libuv_FOUND AND _vendored_libuv_PREFIX)
        set(LIBUV_ROOT_DIR "${_vendored_libuv_PREFIX}" CACHE PATH "" FORCE)
    endif ()
endif ()

if (NOT OPENSSL_ROOT_DIR)
    pkg_check_modules(_vendored_openssl QUIET openssl)
    if (_vendored_openssl_FOUND AND _vendored_openssl_PREFIX)
        set(OPENSSL_ROOT_DIR "${_vendored_openssl_PREFIX}" CACHE PATH "" FORCE)
    endif ()
endif ()

# The driver is C++11 and trips -Werror in this project's warning set. It is
# third-party code we do not lint, so build it with its own defaults.
set(_saved_c_flags "${CMAKE_C_FLAGS}")
set(_saved_cxx_flags "${CMAKE_CXX_FLAGS}")
string(REGEX REPLACE "(^| )-W(error|all|extra|pedantic)([^ ]*)" "" CMAKE_C_FLAGS "${CMAKE_C_FLAGS}")
string(REGEX REPLACE "(^| )-W(error|all|extra|pedantic)([^ ]*)" "" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_subdirectory("${_cass_root}" "${CMAKE_BINARY_DIR}/third-party/cpp-driver" EXCLUDE_FROM_ALL)

set(CMAKE_C_FLAGS "${_saved_c_flags}")
set(CMAKE_CXX_FLAGS "${_saved_cxx_flags}")

if (NOT TARGET cassandra_static)
    message(FATAL_ERROR
        "The vendored driver did not define the 'cassandra_static' target. "
        "third-party/cpp-driver may be a partial import — check its UPSTREAM file.")
endif ()

# clang-tidy and the project warning flags apply to our sources, never to the
# vendored tree.
set_target_properties(cassandra_static PROPERTIES
        C_CLANG_TIDY ""
        CXX_CLANG_TIDY ""
        COMPILE_WARNING_AS_ERROR OFF)

if (NOT TARGET CppDriver::Driver)
    add_library(CppDriver::Driver INTERFACE IMPORTED GLOBAL)
    target_link_libraries(CppDriver::Driver INTERFACE cassandra_static)
    # The driver's public header is cassandra.h under include/.
    target_include_directories(CppDriver::Driver INTERFACE "${_cass_root}/include")
    target_compile_definitions(CppDriver::Driver INTERFACE
            PHP_SCYLLADB_BACKEND_SCYLLA_CPP
            CASS_STATIC)
endif ()

message(STATUS "PHP driver backend: scylla-cpp (vendored third-party/cpp-driver)")

unset(_cass_root)
unset(_saved_c_flags)
unset(_saved_cxx_flags)
