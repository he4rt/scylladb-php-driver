if (BUILD_LIBUV_FROM_SRC)
    if (LINK_LIBUV_STATIC)
        set(LIBUV_BUILD_SHARED OFF)
    else ()
        set(LIBUV_BUILD_SHARED ON)
    endif ()

    CPMAddPackage(
            NAME libuv
            VERSION 1.50.0
            URL https://github.com/libuv/libuv/tarball/v1.50.0
            OPTIONS
            "BUILD_TESTING OFF"
            "BUILD_BENCHMARKS OFF"
            "LIBUV_BUILD_SHARED ${LIBUV_BUILD_SHARED}"
            "CMAKE_C_FLAGS ${CMAKE_C_FLAGS} -fPIC"
            "CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -fPIC"
    )

    set(LIBUV_ROOT_DIR ${libuv_BINARY_DIR})

    if (LINK_LIBUV_STATIC)
        set(LIBUV_LIBRARY "${libuv_BINARY_DIR}/libuv_a.a")
    else ()
        set(LIBUV_LIBRARY "${libuv_BINARY_DIR}/libuv.so")
    endif ()
    set(LIBUV_LIBRARY_DIRS "${libuv_BINARY_DIR}")

    if (LINK_LIBUV_STATIC)
        target_link_libraries(ext_scylladb PRIVATE uv_a)
    else ()
        target_link_libraries(ext_scylladb PRIVATE uv)
    endif ()
else ()
    # First priority: Check for locally built libuv in third-party directory
    set(LIBUV_LOCAL_PATH "${PROJECT_SOURCE_DIR}/third-party/libuv-install")

    # Determine which .pc file to look for based on static/shared setting
    if (LINK_LIBUV_STATIC)
        set(LIBUV_PC_FILE "${LIBUV_LOCAL_PATH}/lib/pkgconfig/libuv-static.pc")
        set(LIBUV_PC_NAME "libuv-static.pc")
    else()
        set(LIBUV_PC_FILE "${LIBUV_LOCAL_PATH}/lib/pkgconfig/libuv.pc")
        set(LIBUV_PC_NAME "libuv.pc")
    endif()

    message(STATUS "Checking for local libuv at: ${LIBUV_LOCAL_PATH}")
    message(STATUS "Looking for: ${LIBUV_PC_FILE} (static: ${LINK_LIBUV_STATIC})")

    if(EXISTS "${LIBUV_PC_FILE}")
        message(STATUS "✓ Found local libuv in third-party directory: ${LIBUV_LOCAL_PATH}")

        # Set PKG_CONFIG_PATH to include our local libuv
        set(ENV{PKG_CONFIG_PATH} "${LIBUV_LOCAL_PATH}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")

        find_package(PkgConfig REQUIRED)
        if (LINK_LIBUV_STATIC)
            target_compile_definitions(ext_scylladb PRIVATE -DUV_STATIC)
            pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv-static)
            message(STATUS "Using static libuv from third-party: ${LIBUV_LIBRARIES}")
        else ()
            pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)
            message(STATUS "Using shared libuv from third-party: ${LIBUV_LIBRARIES}")
        endif ()

        target_link_libraries(ext_scylladb PRIVATE ${LIBUV_LIBRARIES})
        target_link_directories(ext_scylladb PRIVATE ${LIBUV_LIBRARY_DIRS})
        target_include_directories(ext_scylladb PUBLIC ${LIBUV_INCLUDE_DIRS})
    else()
        # Fallback: Try system libuv via pkg-config
        message(STATUS "✗ Local libuv not found at: ${LIBUV_PC_FILE}")
        message(STATUS "Searching for system libuv...")
        message(STATUS "")
        message(STATUS "To build libuv locally, run:")
        message(STATUS "  ./scripts/compile-libuv.sh")
        message(STATUS "Or in CI, the build-dependencies job should build it.")
        message(STATUS "")

        find_package(PkgConfig REQUIRED)
        if (LINK_LIBUV_STATIC)
            target_compile_definitions(ext_scylladb PRIVATE -DUV_STATIC)
            pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv-static)
            message(STATUS "Using static system libuv: ${LIBUV_LIBRARIES}")
        else ()
            pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)
            message(STATUS "Using shared system libuv: ${LIBUV_LIBRARIES}")
        endif ()

        target_link_libraries(ext_scylladb PRIVATE ${LIBUV_LIBRARIES})
        target_link_directories(ext_scylladb PRIVATE ${LIBUV_LIBRARY_DIRS})
        target_include_directories(ext_scylladb PUBLIC ${LIBUV_INCLUDE_DIRS})
    endif()
endif ()