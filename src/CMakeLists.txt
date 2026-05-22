php_driver_module(src ext_scylladb::src)

target_sources(src PRIVATE
        BatchStatement.cpp
        Blob.cpp
        Collection.cpp
        Core.cpp
        Custom.cpp
        ExecutionOptions.cpp
        Future.cpp
        FuturePreparedStatement.cpp
        FutureRows.cpp
        FutureSession.cpp
        FutureClose.cpp
        FutureUtil.cpp
        FutureValue.cpp
        Inet.cpp
        InetUtil.cpp
        Map.cpp
        UuidGen.cpp
        DefaultSession.cpp
        PreparedStatement.cpp
        Session.cpp
        Set.cpp
        SimpleStatement.cpp
        Statement.cpp
        Tuple.cpp
        Type.cpp
        UserTypeValue.cpp
        Uuid.cpp
        UuidInterface.cpp
        Value.cpp
)
