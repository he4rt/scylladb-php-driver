if (BUILD_LIBUV_FROM_SRC)
    if (LINK_LIBUV_STATIC)
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
    )

    if (LINK_LIBUV_STATIC)
        target_link_libraries(ext_scylladb PRIVATE uv_a)
    else ()
        target_link_libraries(ext_scylladb PRIVATE uv)
    endif ()

    target_include_directories(ext_scylladb PUBLIC "${libuv_SOURCE_DIR}/include")
else ()
    find_package(PkgConfig REQUIRED)

    if (LINK_LIBUV_STATIC)
        target_compile_definitions(ext_scylladb PRIVATE UV_STATIC)
        pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv-static)
        message(STATUS "Using static libuv: ${LIBUV_VERSION}")
    else ()
        pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)
    endif ()

    target_link_libraries(ext_scylladb PRIVATE PkgConfig::LIBUV)
endif ()
