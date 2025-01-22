if (NOT ${CUSTOM_PHP_CONFIG} STREQUAL "")
    message(STATUS "Using custom php config: ${CUSTOM_PHP_CONFIG}")
    if (EXISTS ${CUSTOM_PHP_CONFIG})
        set(PHP_CONFIG_EXECUTABLE ${CUSTOM_PHP_CONFIG})
        set(PHP_CONFIG_FOUND ON)
    else ()
        message(FATAL_ERROR "php-config not found")
        set(PHP_CONFIG_FOUND OFF)
    endif ()
else ()
    message(STATUS "Searching php-config PHP-VERSION: ${PHP_VERSION_FOR_PHP_CONFIG}")

    if (${PHP_VERSION_FOR_PHP_CONFIG} STREQUAL "")
        message(FATAL_ERROR "PHP version not defined: ${PHP_VERSION_FOR_PHP_CONFIG}")
    endif ()

    set(hint "${PROJECT_SOURCE_DIR}/php/${PHP_VERSION_FOR_PHP_CONFIG}")

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

    find_program(
            PHP_CONFIG_EXECUTABLE php-config
            HINTS ${hint} "/root/php" "/usr" "/usr/local" "${PROJECT_SOURCE_DIR}/php"
            PATH_SUFFIXES bin
            REQUIRED
    )
    message(STATUS "Found php-config: ${PHP_CONFIG_EXECUTABLE}")
    set(PHP_CONFIG_EXECUTABLE ${PHP_CONFIG_EXECUTABLE})
    set(PHP_CONFIG_FOUND ON)
endif ()
