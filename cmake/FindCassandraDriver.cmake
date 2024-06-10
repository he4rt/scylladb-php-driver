if (PHP_SCYLLADB_LIBCASSANDRA_FROM_SRC)
    if (PHP_SCYLLADB_LIBCASSANDRA_STATIC)
        set(CASS_BUILD_STATIC ON)
        set(CASS_BUILD_SHARED OFF)
    else ()
        set(CASS_BUILD_STATIC OFF)
        set(CASS_BUILD_SHARED ON)
    endif ()

    if (PHP_SCYLLADB_LIBUV_FROM_SRC)
        CPMAddPackage(
                NAME libcassandra
                GITHUB_REPOSITORY datastax/cpp-driver
                OPTIONS
                "CASS_CPP_STANDARD 17"
                "CASS_BUILD_STATIC ${CASS_BUILD_STATIC}"
                "CASS_BUILD_SHARED ${CASS_BUILD_SHARED}"
                "CASS_USE_STD_ATOMIC ON"
                "CASS_USE_TIMERFD ON"
                "CASS_USE_LIBSSH2 ON"
                "CASS_USE_ZLIB ON"
                "CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -fPIC"
                "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fPIC -Wno-error=redundant-move"
                "LIBUV_LIBRARY ${LIBUV_LIBRARY}"
                "LIBUV_INCLUDE_DIR ${libuv_SOURCE_DIR}/include"
        )
    else ()
        CPMAddPackage(
                NAME libcassandra
                GITHUB_REPOSITORY datastax/cpp-driver
                OPTIONS
                "CASS_CPP_STANDARD 17"
                "CASS_BUILD_STATIC ${CASS_BUILD_STATIC}"
                "CASS_BUILD_SHARED ${CASS_BUILD_SHARED}"
                "CASS_USE_STD_ATOMIC ON"
                "CASS_USE_TIMERFD ON"
                "CASS_USE_LIBSSH2 ON"
                "CASS_USE_ZLIB ON"
                "CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -fPIC"
                "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fPIC -Wno-error=redundant-move"
        )
    endif ()

    if (PHP_SCYLLADB_LIBCASSANDRA_STATIC)
        target_link_libraries(ext_scylladb PRIVATE cassandra_static)
        target_compile_definitions(ext_scylladb PRIVATE -DSCYLLADB_STATIC)
    else ()
        target_link_libraries(ext_scylladb PRIVATE "libcassandra.so")
    endif ()

    target_include_directories(ext_scylladb PUBLIC ${libcassandra_SOURCE_DIR}/include)
    set(CASSANDRA_H ${libcassandra_SOURCE_DIR}/include)
else ()
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
endif ()
