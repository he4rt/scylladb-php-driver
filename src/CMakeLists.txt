add_library(src STATIC)
add_library(ext_scylladb::src ALIAS src)
scylladb_php_library(src OFF "${CPU_TYPE}" "${ENABLE_LTO}")


target_sources(
        src
        PRIVATE
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
        FutureValue.cpp
        Inet.cpp
        Map.cpp
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
