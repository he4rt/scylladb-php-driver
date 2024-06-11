include(CheckIPOSupported)
include(CheckCXXCompilerFlag)
include(CheckCCompilerFlag)

function(scylladb_php_library target enable_sanitizers native_arch lto)
    target_include_directories(
        ${target}
        PUBLIC
        ${PHP_INCLUDES}
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_BINARY_DIR}
        ${PROJECT_SOURCE_DIR}
        ${PHP_INCLUDES}
        ${libscylladb_SOURCE_DIR}/include
        ${LIBGMP_INCLUDE_DIRS}
            ${CASSANDRA_H}
    )

        target_compile_features(${target} PUBLIC cxx_std_20 c_std_23)

    set(CMAKE_CXX_STANDARD 20)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    set(CMAKE_CXX_EXTENSIONS OFF)

    set(CMAKE_C_STANDARD 23)
    set(CMAKE_C_STANDARD_REQUIRED ON)
    set(CMAKE_C_EXTENSIONS OFF)

    target_compile_options(
            ${target}
            PRIVATE
            -fPIC
            -Wall
            -Wextra
            -Wno-long-long
            -Wno-deprecated-declarations
            -Wno-unused-parameter
            -Wno-unused-result
            -Wno-variadic-macros
            -Wno-format
            -pthread
    )

    if (${CMAKE_BUILD_TYPE} STREQUAL "Debug")
        target_compile_definitions(${target} PRIVATE -DDEBUG)
    elseif (${CMAKE_BUILD_TYPE} STREQUAL "RelWithDebInfo" OR ${CMAKE_BUILD_TYPE} STREQUAL "Release")
        target_compile_definitions(${target} PRIVATE -DRELEASE)
    endif ()

    if (enable_sanitizers)
        target_compile_options(${target} PRIVATE -fno-inline -fno-omit-frame-pointer)
        add_sanitize_undefined(${target})
        add_sanitize_address(${target})
    endif ()

    if (${APPLE})
        target_link_options(${target} PRIVATE -undefined dynamic_lookup)
    endif()

    scylladb_php_target_optimize(${target} ${native_arch} ${lto})
endfunction()