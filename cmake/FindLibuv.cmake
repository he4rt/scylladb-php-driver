include_guard(GLOBAL)
include(FindPackageHandleStandardArgs)

if (BUILD_LIBUV_FROM_SRC)
    if (LINK_LIBUV_STATIC)
        set(_libuv_shared OFF)
    else ()
        set(_libuv_shared ON)
    endif ()

    CPMAddPackage(
            NAME libuv
            VERSION 1.48.0
            URL https://github.com/libuv/libuv/tarball/v1.48.0
            OPTIONS
            "BUILD_TESTING OFF"
            "BUILD_BENCHMARKS OFF"
            "LIBUV_BUILD_SHARED ${_libuv_shared}"
    )
    unset(_libuv_shared)

    # ── Wrap CPM target under unified name ────────────────────────────────────
    if (NOT TARGET Libuv::Libuv)
        add_library(Libuv::Libuv INTERFACE IMPORTED GLOBAL)
        if (LINK_LIBUV_STATIC)
            target_link_libraries(Libuv::Libuv INTERFACE uv_a)
        else ()
            target_link_libraries(Libuv::Libuv INTERFACE uv)
        endif ()
        target_include_directories(Libuv::Libuv INTERFACE "${libuv_SOURCE_DIR}/include")
    endif ()
else ()
    find_package(PkgConfig REQUIRED)

    if (LINK_LIBUV_STATIC)
        pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv-static)
    else ()
        pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)
    endif ()

    # ── Wrap pkg-config target under unified name ─────────────────────────────
    if (NOT TARGET Libuv::Libuv)
        add_library(Libuv::Libuv INTERFACE IMPORTED GLOBAL)
        target_link_libraries(Libuv::Libuv INTERFACE PkgConfig::LIBUV)
        if (LINK_LIBUV_STATIC)
            target_compile_definitions(Libuv::Libuv INTERFACE UV_STATIC)
        endif ()
    endif ()
endif ()

find_package_handle_standard_args(Libuv
        REQUIRED_VARS Libuv_FOUND
)
set(Libuv_FOUND TRUE)
