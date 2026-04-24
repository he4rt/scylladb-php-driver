include_guard(GLOBAL)

# ── User-overridable install paths ───────────────────────────────────────────
# Both default to the values detected by FindPHP, but can be overridden for
# packaging, DESTDIR installs, or non-standard PHP setups.

set(PHP_EXTENSION_INSTALL_DIR "${PHP_EXTENSION_RELATIVE_DIR}" CACHE PATH
        "Install destination for cassandra.so \
(relative = honours CMAKE_INSTALL_PREFIX / DESTDIR; \
absolute = installs directly to that path)")

if (PHP_INI_SCAN_DIR)
    cmake_path(RELATIVE_PATH PHP_INI_SCAN_DIR
            BASE_DIRECTORY "${PHP_PREFIX}"
            OUTPUT_VARIABLE _ini_rel
    )
    set(PHP_INI_INSTALL_DIR "${_ini_rel}" CACHE PATH
            "Install destination for cassandra.ini \
(relative = honours CMAKE_INSTALL_PREFIX / DESTDIR; \
absolute = installs directly to that path)")
    unset(_ini_rel)
else ()
    set(PHP_INI_INSTALL_DIR "" CACHE PATH
            "Install destination for cassandra.ini (not auto-detected; set manually)")
    message(STATUS
            "PHP_INI_SCAN_DIR not detected. "
            "Set -DPHP_INI_INSTALL_DIR=<path> to install cassandra.ini automatically.")
endif ()

# ── Set CMAKE_INSTALL_PREFIX to PHP_PREFIX so relative paths work ────────────
# Only when the user has not set an explicit prefix. This makes
# `cmake --install build` install directly into the PHP tree, and
# `DESTDIR=/staging cmake --install build` produce a staging tree rooted at
# /staging/PHP_PREFIX — suitable for .deb / .rpm packaging.
if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${PHP_PREFIX}" CACHE PATH
            "Install prefix (defaults to PHP prefix so extension lands in the right place)"
            FORCE)
    message(STATUS "CMAKE_INSTALL_PREFIX set to PHP prefix: ${PHP_PREFIX}")
endif ()

# ── Generate cassandra.ini from template ─────────────────────────────────────
configure_file(
        "${PROJECT_SOURCE_DIR}/cassandra.ini.in"
        "${PROJECT_BINARY_DIR}/cassandra.ini"
        @ONLY
)

# ── php_extension_install(<target>) ──────────────────────────────────────────
# Wire up install() rules for the extension shared library and its INI file.
# Two CMake install components are created so users can install them separately:
#
#   cmake --install build --component extension   # only the .so
#   cmake --install build --component ini         # only the .ini
#   cmake --install build                         # both
function(php_extension_install target)
    install(TARGETS "${target}"
            LIBRARY
            DESTINATION "${PHP_EXTENSION_INSTALL_DIR}"
            COMPONENT   extension
    )

    if (PHP_INI_INSTALL_DIR)
        install(FILES "${PROJECT_BINARY_DIR}/cassandra.ini"
                DESTINATION "${PHP_INI_INSTALL_DIR}"
                RENAME      "30-cassandra.ini"
                COMPONENT   ini
        )
    endif ()

    # ── Verification target: runs after install ───────────────────────────────
    # `cmake --install build && cmake --build build --target verify-extension`
    if (PHP_BINARY)
        add_custom_target(verify-extension
                COMMAND "${PHP_BINARY}" -r "dl('cassandra.so'); echo 'cassandra extension loaded OK\\n';"
                COMMENT "Verifying cassandra extension loads in PHP"
                VERBATIM
        )
    endif ()
endfunction()
