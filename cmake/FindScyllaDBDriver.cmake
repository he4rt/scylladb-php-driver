
if (PHP_SCYLLADB_LIBSCYLLADB_FROM_SRC)
    if (PHP_SCYLLADB_LIBSCYLLADB_STATIC)
        set(CASS_BUILD_STATIC ON)
        set(CASS_BUILD_SHARED OFF)
    else ()
        set(CASS_BUILD_STATIC OFF)
        set(CASS_BUILD_SHARED ON)
    endif ()

    if (PHP_SCYLLADB_LIBUV_FROM_SRC)
        CPMAddPackage(
                NAME libscylladb
                GITHUB_REPOSITORY "scylladb/cpp-driver"
                GIT_TAG master
                OPTIONS
                "CASS_CPP_STANDARD 17"
                "CASS_BUILD_STATIC ${CASS_BUILD_STATIC}"
                "CASS_BUILD_SHARED ${CASS_BUILD_SHARED}"
                "CASS_USE_STD_ATOMIC ON"
                "CASS_USE_TIMERFD ON"
                "CASS_USE_LIBSSH2 ON"
                "CASS_USE_ZLIB OFF"
                "CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -fPIC"
                "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fPIC -Wno-error=redundant-move"
                "LIBUV_LIBRARY ${LIBUV_LIBRARY}"
                "LIBUV_INCLUDE_DIR ${libuv_SOURCE_DIR}/include"
        )
    else ()
        CPMAddPackage(
                NAME libscylladb
                GITHUB_REPOSITORY "scylladb/cpp-driver"
                GIT_TAG master
                OPTIONS
                "CASS_CPP_STANDARD 17"
                "CASS_BUILD_STATIC ${CASS_BUILD_STATIC}"
                "CASS_BUILD_SHARED ${CASS_BUILD_SHARED}"
                "CASS_USE_STD_ATOMIC ON"
                "CASS_USE_TIMERFD ON"
                "CASS_USE_LIBSSH2 ON"
                "CASS_USE_ZLIB OFF"
                "CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -fPIC"
                "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fPIC -Wno-error=redundant-move"
        )
    endif ()

    if (PHP_SCYLLADB_LIBSCYLLADB_STATIC)
        target_link_libraries(ext_scylladb PRIVATE cassandra_static)
        target_compile_definitions(ext_scylladb PRIVATE -DSCYLLADB_STATIC)
    else ()
        target_link_libraries(ext_scylladb PRIVATE "libscylla-cpp-driver.so")
    endif ()

    target_include_directories(ext_scylladb PUBLIC ${libscylladb_SOURCE_DIR}/include)
else ()
    find_package(PkgConfig REQUIRED)

    if (PHP_SCYLLADB_LIBSCYLLADB_STATIC)
        pkg_check_modules(LIBSCYLLADB REQUIRED IMPORTED_TARGET scylla-cpp-driver_static)
    else ()
        pkg_check_modules(LIBSCYLLADB REQUIRED IMPORTED_TARGET scylla-cpp-driver)
    endif ()

    target_link_libraries(ext_scylladb PRIVATE ${LIBSCYLLADB_LIBRARIES})
    target_link_directories(ext_scylladb PRIVATE ${LIBSCYLLADB_LIBRARY_DIRS})
    target_include_directories(ext_scylladb PUBLIC ${LIBSCYLLADB_INCLUDE_DIRS})
endif ()
