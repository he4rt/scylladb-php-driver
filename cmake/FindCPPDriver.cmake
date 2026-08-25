include_guard(GLOBAL)
include(FindPackageHandleStandardArgs)

find_package(PkgConfig REQUIRED)

include(DriverBackend)

# ── pkg-config lookup ────────────────────────────────────────────────────────
# Each backend ships its own pkg-config module name:
#   cassandra   → cassandra.pc / cassandra_static.pc
#   scylla-cpp  → scylla-cpp-driver.pc / scylla-cpp-driver_static.pc
#   scylla-rust → scylladb.pc / scylladb_static.pc
# cpp-rs-driver also installs a `cassandra.pc` compatibility module, so point
# PKG_CONFIG_PATH (or CMAKE_PREFIX_PATH) at one prefix only.
if (PHP_SCYLLADB_BACKEND STREQUAL "cassandra")
    set(_cpp_driver_label "DataStax cassandra")
    set(_install_hint "cassandra")
    set(_cpp_driver_define PHP_SCYLLADB_BACKEND_CASSANDRA)
    if (PHP_SCYLLADB_STATIC)
        set(_cpp_driver_pc "cassandra_static")
    else ()
        set(_cpp_driver_pc "cassandra")
    endif ()
elseif (PHP_SCYLLADB_BACKEND STREQUAL "scylla-rust")
    set(_cpp_driver_label "ScyllaDB cpp-rs-driver")
    set(_install_hint "scylla-rust")
    set(_cpp_driver_define PHP_SCYLLADB_BACKEND_SCYLLA_RUST)
    if (PHP_SCYLLADB_STATIC)
        set(_cpp_driver_pc "scylladb_static")
    else ()
        set(_cpp_driver_pc "scylladb")
    endif ()
else ()
    set(_cpp_driver_label "ScyllaDB cpp-driver")
    set(_install_hint "scylladb")
    set(_cpp_driver_define PHP_SCYLLADB_BACKEND_SCYLLA_CPP)
    if (PHP_SCYLLADB_STATIC)
        set(_cpp_driver_pc "scylla-cpp-driver_static")
    else ()
        set(_cpp_driver_pc "scylla-cpp-driver")
    endif ()
endif ()

pkg_check_modules(LIBCPPDRIVER QUIET IMPORTED_TARGET "${_cpp_driver_pc}")

if (NOT LIBCPPDRIVER_FOUND)
    if (PHP_SCYLLADB_BACKEND STREQUAL "scylla-rust")
        set(_install_cmd "scripts/compile-cpp-rs-driver.sh --prefix <prefix>")
    else ()
        set(_install_cmd "scripts/compile-cpp-driver.sh --driver ${_install_hint}")
    endif ()
    message(FATAL_ERROR
        "Could not find ${_cpp_driver_label} via pkg-config module '${_cpp_driver_pc}'.\n"
        "Hints:\n"
        "  * Install it with: ${_install_cmd}\n"
        "  * Then point CMake at the install prefix with:\n"
        "      -DCPP_DRIVER_PREFIX=<prefix>   (e.g. /usr/local/${_install_hint})\n"
        "  * Switch backend with: -DPHP_SCYLLADB_BACKEND=cassandra|scylla-cpp|scylla-rust\n"
        "  * To link statically, pass: -DPHP_SCYLLADB_STATIC=ON (expects '${_cpp_driver_pc}.pc')\n"
        "Current PKG_CONFIG_PATH: $ENV{PKG_CONFIG_PATH}"
    )
endif ()

set(_cpp_driver_version "${LIBCPPDRIVER_VERSION}")

# ── Static-link dependencies not declared by the driver's .pc ────────────────
# Linking the driver statically pulls its object code into cassandra.so, so we
# inherit its dependencies. The driver is built with CASS_USE_ZLIB=ON but
# scylla-cpp-driver_static.pc lists neither a Libs.private nor zlib, so nothing
# tells us to link it — cassandra.so ends up referencing inflate/crc32 with no
# libz in DT_NEEDED. That resolves by accident on hosts whose php binary links
# libz, and is the gh-117 failure mode (see scripts/check-module-symbols.sh).
#
# Harmless if the driver was built without zlib: the reference disappears and
# the linker drops the unused entry.
if (PHP_SCYLLADB_STATIC)
    find_package(ZLIB REQUIRED)
endif ()

# ── Create CppDriver::Driver INTERFACE IMPORTED target ───────────────────────
if (NOT TARGET CppDriver::Driver)
    add_library(CppDriver::Driver INTERFACE IMPORTED GLOBAL)
    target_link_libraries(CppDriver::Driver INTERFACE PkgConfig::LIBCPPDRIVER)
    if (PHP_SCYLLADB_STATIC)
        target_link_libraries(CppDriver::Driver INTERFACE ZLIB::ZLIB)
    endif ()
    target_compile_definitions(CppDriver::Driver INTERFACE "${_cpp_driver_define}")
endif ()

message(STATUS "PHP driver backend: ${PHP_SCYLLADB_BACKEND} "
               "(${_cpp_driver_label} ${_cpp_driver_version})")

find_package_handle_standard_args(CPPDriver
        REQUIRED_VARS _cpp_driver_version
        VERSION_VAR   _cpp_driver_version
)

unset(_cpp_driver_version)
unset(_cpp_driver_label)
unset(_cpp_driver_pc)
unset(_cpp_driver_define)
unset(_install_hint)
unset(_install_cmd)
