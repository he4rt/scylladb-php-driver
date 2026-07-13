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

        # libuv-static.pc lists the archive as "-l:libuv.a" (GNU ld name-form).
        # GNU ld resolves it, but Apple's ld64 cannot parse the "-l:name" form
        # and fails with: ld: library ':libuv.a' not found. On Apple, resolve
        # the archive to an absolute path and substitute it into the imported
        # target's link interface so the bare name never reaches the linker.
        if (APPLE)
            find_library(LIBUV_STATIC_ARCHIVE
                    NAMES libuv.a uv_a
                    HINTS ${LIBUV_STATIC_LIBRARY_DIRS} ${LIBUV_LIBRARY_DIRS})
            if (NOT LIBUV_STATIC_ARCHIVE)
                message(FATAL_ERROR
                        "LINK_LIBUV_STATIC=ON but no libuv static archive was found in "
                        "'${LIBUV_STATIC_LIBRARY_DIRS}'. Install a static libuv "
                        "or configure with -DLINK_LIBUV_STATIC=OFF.")
            endif ()
            get_target_property(_libuv_link_libs PkgConfig::LIBUV INTERFACE_LINK_LIBRARIES)
            list(TRANSFORM _libuv_link_libs
                    REPLACE "^(-l)?:?libuv\\.a$" "${LIBUV_STATIC_ARCHIVE}")
            set_target_properties(PkgConfig::LIBUV PROPERTIES
                    INTERFACE_LINK_LIBRARIES "${_libuv_link_libs}")
            unset(_libuv_link_libs)
        endif ()
    else ()
        pkg_check_modules(LIBUV REQUIRED IMPORTED_TARGET libuv)
    endif ()

    # ── Wrap pkg-config target under unified name ─────────────────────────────
    # PkgConfig::LIBUV carries -l:libuv.a (name-form for static libs) but
    # INTERFACE_LINK_DIRECTORIES may not propagate through an INTERFACE wrapper.
    # Explicitly add the directory so the linker can resolve -l:libuv.a.
    if (NOT TARGET Libuv::Libuv)
        add_library(Libuv::Libuv INTERFACE IMPORTED GLOBAL)
        target_link_libraries(Libuv::Libuv INTERFACE PkgConfig::LIBUV)
        target_link_directories(Libuv::Libuv INTERFACE ${LIBUV_LIBRARY_DIRS})
        if (LINK_LIBUV_STATIC)
            target_compile_definitions(Libuv::Libuv INTERFACE UV_STATIC)
        endif ()
    endif ()
endif ()

set(Libuv_FOUND TRUE)
find_package_handle_standard_args(Libuv
        REQUIRED_VARS Libuv_FOUND
)
