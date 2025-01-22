add_library(exceptions STATIC)
add_library(ext_scylladb::exceptions ALIAS exceptions)
scylladb_php_library(exceptions OFF "${CPU_TYPE}" "${ENABLE_LTO}")


target_sources(
        exceptions
        PRIVATE
        Exception.cpp
        AlreadyExistsException.cpp
        AuthenticationException.cpp
        ConfigurationException.cpp
        DivideByZeroException.cpp
        DomainException.cpp
        ExecutionException.cpp
        InvalidArgumentException.cpp
        InvalidQueryException.cpp
        InvalidSyntaxException.cpp
        IsBootstrappingException.cpp
        LogicException.cpp
        OverloadedException.cpp
        ProtocolException.cpp
        RangeException.cpp
        ReadTimeoutException.cpp
        RuntimeException.cpp
        ServerException.cpp
        TimeoutException.cpp
        TruncateException.cpp
        UnauthorizedException.cpp
        UnavailableException.cpp
        UnpreparedException.cpp
        ValidationException.cpp
        WriteTimeoutException.cpp
)