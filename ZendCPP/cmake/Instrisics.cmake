include(CheckCXXCompilerFlag)

function(target_use_intrinsics target)
    CHECK_CXX_COMPILER_FLAG("-msse4.1" COMPILER_SUPPORTS_SSE4_1)
    CHECK_CXX_COMPILER_FLAG("-mavx2" COMPILER_SUPPORTS_AVX2)
    CHECK_CXX_COMPILER_FLAG("-mavx512f" COMPILER_SUPPORTS_AVX_512)

    if (COMPILER_SUPPORTS_SSE4_1)
        message(STATUS "Compiler supports SSE4.1")
        target_compile_options(${target} PRIVATE -msse4.1)
        target_compile_definitions(${target} PRIVATE ZENDCPP_SUPPORTS_SSE41)
    endif ()

    if (COMPILER_SUPPORTS_AVX2)
        message(STATUS "Compiler supports AVX2")
        target_compile_options(${target} PRIVATE -mavx2)
        target_compile_definitions(${target} PRIVATE ZENDCPP_SUPPORTS_AVX2)
    endif ()

    if (COMPILER_SUPPORTS_AVX_512)
        message(STATUS "Compiler supports AVX512")
        target_compile_options(${target} PRIVATE -mavx512f)
        target_compile_definitions(${target} PRIVATE ZENDCPP_SUPPORTS_AVX512)
    endif ()

endfunction()