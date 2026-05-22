php_scylladb_module(src ext_scylladb::src)

target_sources(src PRIVATE
        BatchStatement.c
        Blob.c
        Collection.c
        Core.c
        Custom.c
        ExecutionOptions.c
        FuturePreparedStatement.c
        FutureRows.c
        FutureSession.c
        FutureClose.c
        FutureUtil.c
        FutureValue.c
        Inet.c
        InetUtil.c
        Map.c
        UuidGen.c
        DefaultSession.c
        PreparedStatement.c
        Set.c
        SimpleStatement.c
        Tuple.c
        Type.c
        UserTypeValue.c
        Uuid.c
)


php_scylladb_generate_arginfo(src
        BatchStatement.stub.php
        Inet.stub.php
        Blob.stub.php
        Map.stub.php
        Cassandra.stub.php
        PreparedStatement.stub.php
        Collection.stub.php
        Session.stub.php
        DefaultSession.stub.php
        Set.stub.php
        ExecutionOptions.stub.php
        SimpleStatement.stub.php
        Future.stub.php
        Statement.stub.php
        FutureClose.stub.php
        Tuple.stub.php
        FuturePreparedStatement.stub.php
        UserTypeValue.stub.php
        FutureRows.stub.php
        Uuid.stub.php
        FutureSession.stub.php
        UuidInterface.stub.php
        FutureValue.stub.php
        Value.stub.php
        # Type.cpp at this level #includes "Type/Type_arginfo.h" — needs to
        # be a generated-file dep of THIS target, not just the `type` target.
        Type/Type.stub.php
)

php_scylladb_generate_descriptor(src
        Type/Type.stub.php
        Value.stub.php
        UuidInterface.stub.php
        Blob.stub.php
        Inet.stub.php
        Uuid.stub.php
        Set.stub.php
        Map.stub.php
        Collection.stub.php
        Tuple.stub.php
        UserTypeValue.stub.php
        Cassandra.stub.php
        Future.stub.php
        FuturePreparedStatement.stub.php
        FutureRows.stub.php
        FutureSession.stub.php
        FutureValue.stub.php
        FutureClose.stub.php
        Session.stub.php
        DefaultSession.stub.php
        Statement.stub.php
        SimpleStatement.stub.php
        PreparedStatement.stub.php
        BatchStatement.stub.php
        ExecutionOptions.stub.php
)
