find_package(PkgConfig REQUIRED)

if (PHP_SCYLLADB_LIBCASSANDRA_STATIC)
    pkg_check_modules(LIBCASSANDRA REQUIRED IMPORTED_TARGET cassandra_static)
else ()
    pkg_check_modules(LIBCASSANDRA REQUIRED IMPORTED_TARGET cassandra)
endif ()

target_link_libraries(ext_scylladb PRIVATE ${LIBCASSANDRA_LIBRARIES})
target_link_directories(ext_scylladb PRIVATE ${LIBCASSANDRA_LIBRARY_DIRS})
target_include_directories(ext_scylladb PUBLIC ${LIBCASSANDRA_INCLUDE_DIRS})
set(CASSANDRA_H ${LIBCASSANDRA_INCLUDE_DIRS})
