include_guard(GLOBAL)
include(FindPackageHandleStandardArgs)

find_package(PkgConfig REQUIRED)

# Pick driver + linkage → pkg-config module name.
if (USE_LIBCASSANDRA)
    set(_cpp_driver_label "DataStax cassandra")
    if (PHP_DRIVER_STATIC)
        set(_cpp_driver_pc "cassandra_static")
    else ()
        set(_cpp_driver_pc "cassandra")
    endif ()
else ()
    set(_cpp_driver_label "ScyllaDB cpp-driver")
    if (PHP_DRIVER_STATIC)
        set(_cpp_driver_pc "scylla-cpp-driver_static")
    else ()
        set(_cpp_driver_pc "scylla-cpp-driver")
    endif ()
endif ()

pkg_check_modules(LIBCPPDRIVER QUIET IMPORTED_TARGET "${_cpp_driver_pc}")

if (NOT LIBCPPDRIVER_FOUND)
    if (USE_LIBCASSANDRA)
        set(_install_hint "cassandra")
    else ()
        set(_install_hint "scylladb")
    endif ()
    message(FATAL_ERROR
        "Could not find ${_cpp_driver_label} via pkg-config module '${_cpp_driver_pc}'.\n"
        "Hints:\n"
        "  * Install it with: scripts/compile-cpp-driver.sh --driver ${_install_hint}\n"
        "  * Then point CMake at the install prefix with:\n"
        "      -DCPP_DRIVER_PREFIX=<prefix>   (e.g. /usr/local/${_install_hint})\n"
        "  * To use the DataStax driver instead, pass: -DUSE_LIBCASSANDRA=ON\n"
        "  * To link statically, pass: -DPHP_DRIVER_STATIC=ON (expects '${_cpp_driver_pc}.pc')\n"
        "Current PKG_CONFIG_PATH: $ENV{PKG_CONFIG_PATH}"
    )
endif ()

set(_cpp_driver_version "${LIBCPPDRIVER_VERSION}")

# ── Create CppDriver::Driver INTERFACE IMPORTED target ───────────────────────
if (NOT TARGET CppDriver::Driver)
    add_library(CppDriver::Driver INTERFACE IMPORTED GLOBAL)
    target_link_libraries(CppDriver::Driver INTERFACE PkgConfig::LIBCPPDRIVER)
endif ()

find_package_handle_standard_args(CPPDriver
        REQUIRED_VARS _cpp_driver_version
        VERSION_VAR   _cpp_driver_version
)

unset(_cpp_driver_version)
unset(_cpp_driver_label)
unset(_cpp_driver_pc)
