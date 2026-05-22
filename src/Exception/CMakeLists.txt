php_scylladb_module(exceptions ext_scylladb::exceptions)

target_sources(exceptions PRIVATE
        exceptions.c
)

php_scylladb_generate_arginfo(exceptions
        exceptions.stub.php
)
