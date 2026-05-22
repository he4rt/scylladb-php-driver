php_driver_module(database ext_scylladb::database)

target_sources(database PRIVATE
        Aggregate.cpp
        Column.cpp
        Function.cpp
        Index.cpp
        Keyspace.cpp
        MaterializedView.cpp
        ResultDecoder.cpp
        Rows.cpp
        Schema.cpp
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
