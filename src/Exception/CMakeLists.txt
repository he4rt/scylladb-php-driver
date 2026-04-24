php_driver_module(exceptions ext_scylladb::exceptions)

target_sources(exceptions PRIVATE
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
