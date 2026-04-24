include_guard(GLOBAL)
include(FindPackageHandleStandardArgs)

find_package(PkgConfig REQUIRED)

if (USE_LIBCASSANDRA)
    if (PHP_DRIVER_STATIC)
        pkg_check_modules(LIBCASSANDRA REQUIRED IMPORTED_TARGET cassandra_static)
    else ()
        pkg_check_modules(LIBCASSANDRA REQUIRED IMPORTED_TARGET cassandra)
    endif ()
    set(_cpp_driver_underlying PkgConfig::LIBCASSANDRA)
    set(_cpp_driver_version    "${LIBCASSANDRA_VERSION}")
else ()
    if (PHP_DRIVER_STATIC)
        pkg_check_modules(LIBSCYLLADB REQUIRED IMPORTED_TARGET scylla-cpp-driver_static)
    else ()
        pkg_check_modules(LIBSCYLLADB REQUIRED IMPORTED_TARGET scylla-cpp-driver)
    endif ()
    set(_cpp_driver_underlying PkgConfig::LIBSCYLLADB)
    set(_cpp_driver_version    "${LIBSCYLLADB_VERSION}")
endif ()

# ── Create CppDriver::Driver INTERFACE IMPORTED target ───────────────────────
if (NOT TARGET CppDriver::Driver)
    add_library(CppDriver::Driver INTERFACE IMPORTED GLOBAL)
    target_link_libraries(CppDriver::Driver INTERFACE "${_cpp_driver_underlying}")
endif ()
unset(_cpp_driver_underlying)

find_package_handle_standard_args(CPPDriver
        REQUIRED_VARS _cpp_driver_version
        VERSION_VAR   _cpp_driver_version
)
unset(_cpp_driver_version)
