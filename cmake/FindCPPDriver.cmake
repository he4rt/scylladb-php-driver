include_guard(GLOBAL)
include(FindPackageHandleStandardArgs)

find_package(PkgConfig REQUIRED)

# ── Backend selection ────────────────────────────────────────────────────────
# PHP_DRIVER_BACKEND is the canonical knob. The legacy USE_LIBCASSANDRA flag is
# honoured for one release as a fallback: ON → "cassandra", OFF → "scylla-cpp".
# Set PHP_DRIVER_BACKEND explicitly to override.
set(_php_driver_backend_default "scylla-cpp")
if (DEFINED USE_LIBCASSANDRA)
    if (USE_LIBCASSANDRA)
        set(_php_driver_backend_default "cassandra")
    else ()
        set(_php_driver_backend_default "scylla-cpp")
    endif ()
endif ()

set(PHP_DRIVER_BACKEND "${_php_driver_backend_default}" CACHE STRING
        "C/C++ driver backend: cassandra | scylla-cpp | scylla-rust")
set_property(CACHE PHP_DRIVER_BACKEND PROPERTY STRINGS cassandra scylla-cpp scylla-rust)
unset(_php_driver_backend_default)

if (NOT PHP_DRIVER_BACKEND MATCHES "^(cassandra|scylla-cpp|scylla-rust)$")
    message(FATAL_ERROR
        "PHP_DRIVER_BACKEND='${PHP_DRIVER_BACKEND}' is invalid. "
        "Expected: cassandra, scylla-cpp, or scylla-rust.")
endif ()

# ── pkg-config lookup ────────────────────────────────────────────────────────
# Note: cpp-rs-driver and scylla cpp-driver both ship a pkg-config module named
# `scylla-cpp-driver`. The user disambiguates by pointing PKG_CONFIG_PATH (or
# CMAKE_PREFIX_PATH) at the install prefix of the desired backend before
# configuring. We additionally probe a sentinel symbol so a misconfigured
# environment fails loudly instead of linking the wrong driver.
if (PHP_DRIVER_BACKEND STREQUAL "cassandra")
    if (PHP_DRIVER_STATIC)
        pkg_check_modules(LIBCASSANDRA REQUIRED IMPORTED_TARGET cassandra_static)
    else ()
        pkg_check_modules(LIBCASSANDRA REQUIRED IMPORTED_TARGET cassandra)
    endif ()
    set(_cpp_driver_underlying PkgConfig::LIBCASSANDRA)
    set(_cpp_driver_version    "${LIBCASSANDRA_VERSION}")
    set(_cpp_driver_define     PHP_DRIVER_BACKEND_CASSANDRA)
else ()
    # Both scylla-cpp and scylla-rust ship scylla-cpp-driver{,_static}.pc
    if (PHP_DRIVER_STATIC)
        pkg_check_modules(LIBSCYLLADB REQUIRED IMPORTED_TARGET scylla-cpp-driver_static)
    else ()
        pkg_check_modules(LIBSCYLLADB REQUIRED IMPORTED_TARGET scylla-cpp-driver)
    endif ()
    set(_cpp_driver_underlying PkgConfig::LIBSCYLLADB)
    set(_cpp_driver_version    "${LIBSCYLLADB_VERSION}")
    if (PHP_DRIVER_BACKEND STREQUAL "scylla-rust")
        set(_cpp_driver_define PHP_DRIVER_BACKEND_SCYLLA_RUST)
    else ()
        set(_cpp_driver_define PHP_DRIVER_BACKEND_SCYLLA_CPP)
    endif ()
endif ()

# ── Create CppDriver::Driver INTERFACE IMPORTED target ───────────────────────
if (NOT TARGET CppDriver::Driver)
    add_library(CppDriver::Driver INTERFACE IMPORTED GLOBAL)
    target_link_libraries(CppDriver::Driver INTERFACE "${_cpp_driver_underlying}")
    target_compile_definitions(CppDriver::Driver INTERFACE "${_cpp_driver_define}")
endif ()

message(STATUS "PHP driver backend: ${PHP_DRIVER_BACKEND} (cass library version ${_cpp_driver_version})")

unset(_cpp_driver_underlying)
unset(_cpp_driver_define)

find_package_handle_standard_args(CPPDriver
        REQUIRED_VARS _cpp_driver_version
        VERSION_VAR   _cpp_driver_version
)
unset(_cpp_driver_version)
