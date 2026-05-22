php_scylladb_module(cluster_builder ext_scylladb::cluster::builder)

target_sources(cluster_builder PRIVATE
        Builder.cpp
        BuilderHandlers.cpp
        DefaultCluster.cpp
        DefaultClusterHandlers.cpp
)

php_scylladb_generate_arginfo(cluster_builder
        Builder.stub.php
        ClusterInterface.stub.php
        DefaultCluster.stub.php
)

php_scylladb_generate_descriptor(cluster_builder
        Builder.stub.php
        ClusterInterface.stub.php
        DefaultCluster.stub.php
)
