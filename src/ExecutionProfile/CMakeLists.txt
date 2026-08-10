php_scylladb_module(execution_profile ext_scylladb::execution_profile)

target_sources(execution_profile PRIVATE
        ExecutionProfile.c
)

php_scylladb_generate_arginfo(execution_profile
        ExecutionProfile.stub.php
)

php_scylladb_generate_descriptor(execution_profile
        ExecutionProfile.stub.php
)
