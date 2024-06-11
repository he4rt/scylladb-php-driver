find_package(PkgConfig REQUIRED)

if (PHP_SCYLLADB_LIBSCYLLADB_STATIC)
    pkg_check_modules(LIBSCYLLADB REQUIRED IMPORTED_TARGET scylla-cpp-driver_static)
else ()
    pkg_check_modules(LIBSCYLLADB REQUIRED IMPORTED_TARGET scylla-cpp-driver)
endif ()

target_include_directories(ext_scylladb PUBLIC "${LIBSCYLLADB_INCLUDE_DIRS}")
target_link_libraries(ext_scylladb PRIVATE "${LIBSCYLLADB_LIBRARIES}")
target_link_directories(ext_scylladb PRIVATE "${LIBSCYLLADB_LIBRARY_DIRS}")
set(CASSANDRA_H ${LIBSCYLLADB_INCLUDE_DIRS})
