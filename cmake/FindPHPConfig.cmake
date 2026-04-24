if (NOT CUSTOM_PHP_CONFIG STREQUAL "")
    message(STATUS "Using custom php-config: ${CUSTOM_PHP_CONFIG}")
    set(PHP_CONFIG_EXECUTABLE "${CUSTOM_PHP_CONFIG}")
    set(PHP_CONFIG_FOUND ON)
else ()
    message(STATUS "Searching for php-config (PHP ${PHP_VERSION_FOR_PHP_CONFIG})")

    if (PHP_VERSION_FOR_PHP_CONFIG STREQUAL "")
        message(FATAL_ERROR "PHP_VERSION_FOR_PHP_CONFIG is not set")
    endif ()

    set(_php_hint "${PROJECT_SOURCE_DIR}/php/${PHP_VERSION_FOR_PHP_CONFIG}")

    if (PHP_DEBUG)
        set(_php_hint "${_php_hint}-debug")
    else ()
        set(_php_hint "${_php_hint}-release")
    endif ()

    if (PHP_THREAD_SAFE)
        set(_php_hint "${_php_hint}-zts")
    else ()
        set(_php_hint "${_php_hint}-nts")
    endif ()

    find_program(
            PHP_CONFIG_EXECUTABLE php-config
            HINTS "${_php_hint}" "/usr" "/usr/local" "${PROJECT_SOURCE_DIR}/php"
            PATH_SUFFIXES bin
            REQUIRED
    )

    unset(_php_hint)
    message(STATUS "Found php-config: ${PHP_CONFIG_EXECUTABLE}")
    set(PHP_CONFIG_FOUND ON)
endif ()
