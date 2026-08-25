include_guard(GLOBAL)

# Normalises PHP_SCYLLADB_BACKEND. Included by the root CMakeLists.txt before it
# chooses between the vendored driver and the pkg-config lookup, and by
# cmake/FindCPPDriver.cmake so that module still works on its own.

# PHP_SCYLLADB_BACKEND is the canonical knob. The legacy USE_LIBCASSANDRA flag is
# honoured for one release as a fallback: ON → "cassandra", OFF → "scylla-cpp".
# Set PHP_SCYLLADB_BACKEND explicitly to override.
set(_php_scylladb_backend_default "scylla-cpp")
if (DEFINED USE_LIBCASSANDRA)
    if (USE_LIBCASSANDRA)
        set(_php_scylladb_backend_default "cassandra")
    else ()
        set(_php_scylladb_backend_default "scylla-cpp")
    endif ()
endif ()

set(PHP_SCYLLADB_BACKEND "${_php_scylladb_backend_default}" CACHE STRING
        "C/C++ driver backend: cassandra | scylla-cpp | scylla-rust")
set_property(CACHE PHP_SCYLLADB_BACKEND PROPERTY STRINGS cassandra scylla-cpp scylla-rust)
unset(_php_scylladb_backend_default)

if (NOT PHP_SCYLLADB_BACKEND MATCHES "^(cassandra|scylla-cpp|scylla-rust)$")
    message(FATAL_ERROR
        "PHP_SCYLLADB_BACKEND='${PHP_SCYLLADB_BACKEND}' is invalid. "
        "Expected: cassandra, scylla-cpp, or scylla-rust.")
endif ()
