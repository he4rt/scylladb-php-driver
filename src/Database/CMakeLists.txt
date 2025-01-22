add_library(database STATIC)
add_library(ext_scylladb::database ALIAS database)

scylladb_php_library(database OFF "${CPU_TYPE}" "${ENABLE_LTO}")


target_sources(
        database
        PRIVATE
        Aggregate.cpp
        Column.cpp
        Function.cpp
        Index.cpp
        Keyspace.cpp
        MaterializedView.cpp
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

