php_driver_module(cluster_builder ext_scylladb::cluster::builder)

target_sources(cluster_builder PRIVATE
        Builder.cpp
        BuilderHandlers.cpp
        Cluster.cpp
        DefaultCluster.cpp
        DefaultClusterHandlers.cpp
)
