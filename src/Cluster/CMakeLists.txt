add_library(cluster_builder STATIC BuilderHandlers.h)
add_library(ext_scylladb::cluster::builder ALIAS cluster_builder)

scylladb_php_library(cluster_builder OFF "${CPU_TYPE}" "${ENABLE_LTO}")

target_sources(
        cluster_builder
        PRIVATE
        Builder.cpp
        BuilderHandlers.cpp
        Cluster.cpp
        DefaultCluster.cpp
        DefaultClusterHandlers.cpp
)
