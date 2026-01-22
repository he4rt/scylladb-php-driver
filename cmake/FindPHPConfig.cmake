if (NOT ${CUSTOM_PHP_CONFIG} STREQUAL "")
    message(STATUS "Using custom php config: ${CUSTOM_PHP_CONFIG}")
    set(PHP_CONFIG_EXECUTABLE ${CUSTOM_PHP_CONFIG})
    set(PHP_CONFIG_FOUND ON)
else ()
    # Unset to force fresh search
    unset(PHP_CONFIG_EXECUTABLE CACHE)

    # First priority: Search in local third-party installation
    message(STATUS "=== Searching for php-config ===")

    if (NOT ${PHP_VERSION_FOR_PHP_CONFIG} STREQUAL "")
        set(hint "${PROJECT_SOURCE_DIR}/third-party/php/${PHP_VERSION_FOR_PHP_CONFIG}")

        if (${PHP_DEBUG})
            set(hint "${hint}-debug")
        else ()
            set(hint "${hint}-release")
        endif ()

        if (${PHP_THREAD_SAFE})
            set(hint "${hint}-zts")
        else ()
            set(hint "${hint}-nts")
        endif ()

        message(STATUS "Looking for local PHP at: ${hint}/bin/php-config")
    else ()
        set(hint "${PROJECT_SOURCE_DIR}/third-party/php")
        message(STATUS "Looking for local PHP in: ${hint}")
    endif ()

    message(STATUS "Also searching in:")
    message(STATUS "  - ${PROJECT_SOURCE_DIR}/third-party/php")
    message(STATUS "  - ${PROJECT_SOURCE_DIR}/php")

    find_program(
            PHP_CONFIG_EXECUTABLE php-config
            HINTS ${hint} "${PROJECT_SOURCE_DIR}/third-party/php" "${PROJECT_SOURCE_DIR}/php"
            PATH_SUFFIXES bin
            NO_DEFAULT_PATH
    )

    if (PHP_CONFIG_EXECUTABLE)
        message(STATUS "✓ Found php-config in local installation: ${PHP_CONFIG_EXECUTABLE}")
        set(PHP_CONFIG_FOUND ON)
    else ()
        # Second priority: Search in system installation
        message(STATUS "✗ Local php-config not found")
        message(STATUS "Checked:")
        if (NOT ${PHP_VERSION_FOR_PHP_CONFIG} STREQUAL "")
            message(STATUS "  - ${hint}/bin/php-config")
        endif()
        message(STATUS "  - ${PROJECT_SOURCE_DIR}/third-party/php/*/bin/php-config")
        message(STATUS "  - ${PROJECT_SOURCE_DIR}/php/*/bin/php-config")
        message(STATUS "")
        message(STATUS "Searching for system php-config...")

        find_program(PHP_CONFIG_EXECUTABLE php-config)

        if (PHP_CONFIG_EXECUTABLE)
            message(STATUS "✓ Found system php-config: ${PHP_CONFIG_EXECUTABLE}")
            set(PHP_CONFIG_FOUND ON)
        else ()
            # Nothing found - FAIL
            message(STATUS "")
            message(FATAL_ERROR "php-config not found in local installation or system.
Please install PHP development files or set CUSTOM_PHP_CONFIG to specify the path.

For local development:
  - System: Install php-dev package (apt/brew/dnf install php-dev)
  - Custom: cmake -DCUSTOM_PHP_CONFIG=/path/to/php-config

For third-party build:
  - Build PHP in third-party/php/\${VERSION}-debug-nts/
  - Or use system PHP (already available via shivammathur/setup-php in CI)")
        endif ()
    endif ()
endif ()
