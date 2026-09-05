php_scylladb_module(database ext_scylladb::database)

target_sources(database PRIVATE
        ResultDecoder.c
        Rows.c
        Table.c
        DefaultColumn.c
        DefaultKeyspace.c
        DefaultMaterializedView.c
        DefaultSchema.c
        DefaultTable.c
)

set(_database_stubs
        Column.stub.php
        DefaultColumn.stub.php
        DefaultKeyspace.stub.php
        DefaultMaterializedView.stub.php
        DefaultSchema.stub.php
        DefaultTable.stub.php
        Keyspace.stub.php
        MaterializedView.stub.php
        Rows.stub.php
        Schema.stub.php
        Table.stub.php
)

if (PHP_SCYLLADB_ENABLE_LEGACY_SCHEMA_META)
    target_sources(database PRIVATE
            DefaultAggregate.c
            DefaultFunction.c
            DefaultIndex.c
    )

    list(APPEND _database_stubs
            Aggregate.stub.php
            DefaultAggregate.stub.php
            Function_.stub.php
            DefaultFunction.stub.php
            Index.stub.php
            DefaultIndex.stub.php
    )
endif ()

php_scylladb_generate_arginfo(database ${_database_stubs})
php_scylladb_generate_descriptor(database ${_database_stubs})
