add_library(src STATIC)
add_library(ext_scylladb::src ALIAS src)
scylladb_php_library(src OFF ${PHP_SCYLLADB_OPTIMISE_FOR_CURRENT_MACHINE} ON)

target_sources(
        src
        PRIVATE
        BatchStatement.cpp
        Blob.cpp
        Cassandra.cpp
        Custom.cpp
        ExecutionOptions.cpp
        Future.cpp
        FuturePreparedStatement.cpp
        FutureRows.cpp
        FutureSession.cpp
        FutureClose.cpp
        FutureValue.cpp
        Inet.cpp
        DefaultSession.cpp
        PreparedStatement.cpp
        Session.cpp
        SimpleStatement.cpp
        Statement.cpp
        Type.cpp
        UserTypeValue.cpp
        Uuid.cpp
        UuidInterface.cpp
        Value.cpp
)
