php_scylladb_module(cluster_builder ext_scylladb::cluster::builder)

target_sources(cluster_builder PRIVATE
        Builder.c
        BuilderHandlers.c
        DefaultCluster.c
        DefaultClusterHandlers.c
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
