php_scylladb_module(database ext_scylladb::database)

target_sources(database PRIVATE
        ResultDecoder.cpp
        Rows.cpp
        Table.cpp
        DefaultAggregate.cpp
        DefaultColumn.cpp
        DefaultFunction.cpp
        DefaultIndex.cpp
        DefaultKeyspace.cpp
        DefaultMaterializedView.cpp
        DefaultSchema.cpp
        DefaultTable.cpp
)

php_scylladb_generate_arginfo(database
        Aggregate.stub.php
        Column.stub.php
        DefaultAggregate.stub.php
        DefaultColumn.stub.php
        DefaultFunction.stub.php
        DefaultIndex.stub.php
        DefaultKeyspace.stub.php
        DefaultMaterializedView.stub.php
        DefaultSchema.stub.php
        DefaultTable.stub.php
        Function_.stub.php
        Index.stub.php
        Keyspace.stub.php
        MaterializedView.stub.php
        Rows.stub.php
        Schema.stub.php
        Table.stub.php
)

php_scylladb_generate_descriptor(database
        Aggregate.stub.php
        Column.stub.php
        DefaultAggregate.stub.php
        DefaultColumn.stub.php
        DefaultFunction.stub.php
        DefaultIndex.stub.php
        DefaultKeyspace.stub.php
        DefaultMaterializedView.stub.php
        DefaultSchema.stub.php
        DefaultTable.stub.php
        Function_.stub.php
        Index.stub.php
        Keyspace.stub.php
        MaterializedView.stub.php
        Rows.stub.php
        Schema.stub.php
        Table.stub.php
)
