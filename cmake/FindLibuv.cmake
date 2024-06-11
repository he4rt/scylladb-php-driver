if (PHP_SCYLLADB_LIBUV_FROM_SRC)
    if (PHP_SCYLLADB_LIBUV_STATIC)
        set(LIBUV_BUILD_SHARED OFF)
    else ()
        set(LIBUV_BUILD_SHARED ON)
    endif ()

    CPMAddPackage(
            NAME libuv
            VERSION 1.48.0
            URL https://github.com/libuv/libuv/tarball/v1.48.0
            OPTIONS
            "BUILD_TESTING OFF"
            "BUILD_BENCHMARKS OFF"
            "LIBUV_BUILD_SHARED ${LIBUV_BUILD_SHARED}"
            "CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -fPIC"
            "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fPIC"
    )

    set(LIBUV_ROOT_DIR ${libuv_BINARY_DIR})

    if (PHP_SCYLLADB_LIBUV_STATIC)
        set(LIBUV_LIBRARY "${libuv_BINARY_DIR}/libuv_a.a")
    else ()
        set(LIBUV_LIBRARY "${libuv_BINARY_DIR}/libuv.so")
    endif ()
    set(LIBUV_LIBRARY_DIRS "${libuv_BINARY_DIR}")

    if (PHP_SCYLLADB_LIBUV_STATIC)
        target_link_libraries(ext_scylladb PRIVATE uv_a)
    else ()
        target_link_libraries(ext_scylladb PRIVATE uv)
    endif ()
else ()
    find_package(PkgConfig REQUIRED)
    if (PHP_SCYLLADB_LIBUV_STATIC)
        target_compile_definitions(ext_scylladb PRIVATE -DUV_STATIC)
        pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv-static)
    else ()
        pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)
    endif ()

    target_link_libraries(ext_scylladb PRIVATE ${LIBUV_LIBRARIES})
    target_link_directories(ext_scylladb PRIVATE ${LIBUV_LIBRARY_DIRS})
    target_include_directories(ext_scylladb PUBLIC ${LIBUV_INCLUDE_DIRS})
endif ()