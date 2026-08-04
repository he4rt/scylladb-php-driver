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

    # ── Verification target ───────────────────────────────────────────────────
    # `cmake --build build --target verify-extension`
    #
    # Two checks on the freshly built module, not the installed copy:
    #
    # 1. A static dependency audit (scripts/check-module-symbols.sh) — every
    #    symbol the module references must be provided by a library it actually
    #    declares, or by PHP itself. This is the real gh-117 guard: it depends
    #    on nothing about the machine running it.
    #
    # 2. An isolated load. -n ignores php.ini so no other extension is loaded
    #    first (PHP dlopen()s extensions RTLD_GLOBAL, so an earlier ext/gmp
    #    silently donates the mpz_* symbols), and LD_BIND_NOW forces the eager
    #    binding musl does by default, instead of glibc's lazy binding which
    #    defers a missing dependency until first call. Weaker than check 1 —
    #    a php binary that itself links libgmp masks the fault — but it also
    #    catches failures a symbol audit cannot, such as a broken MINIT.
    if (PHP_BINARY)
        # macOS links PHP extensions with -undefined dynamic_lookup, so
        # unresolved symbols are expected there and eager binding is meaningless.
        if (APPLE)
            set(_verify_launcher)
        else ()
            set(_verify_launcher "${CMAKE_COMMAND}" -E env LD_BIND_NOW=1)
        endif ()

        # cpp-rs-driver does not implement the cass_*_meta_* schema APIs the
        # extension calls, so that backend has unresolved symbols no matter how
        # it is linked. Report them, but do not fail a build that is already
        # marked experimental in CI.
        if (PHP_SCYLLADB_BACKEND STREQUAL "scylla-rust")
            set(_verify_symbol_mode "--warn-only")
        else ()
            set(_verify_symbol_mode "")
        endif ()

        add_custom_target(verify-extension
                COMMAND "${CMAKE_COMMAND}" -E env sh
                        "${PROJECT_SOURCE_DIR}/scripts/check-module-symbols.sh"
                        "$<TARGET_FILE:${target}>" "${PHP_BINARY}"
                        ${_verify_symbol_mode}
                COMMAND ${_verify_launcher}
                        "${PHP_BINARY}" -n
                        -d "extension=$<TARGET_FILE:${target}>"
                        -r "if (!extension_loaded('cassandra')) { fwrite(STDERR, 'cassandra extension failed to load' . PHP_EOL); exit(1); } echo 'cassandra extension loaded OK', PHP_EOL;"
                COMMENT "Verifying cassandra extension loads (no php.ini, eager symbol binding)"
                VERBATIM
        )
        add_dependencies(verify-extension "${target}")
        unset(_verify_launcher)
        unset(_verify_symbol_mode)
    endif ()
endfunction()
