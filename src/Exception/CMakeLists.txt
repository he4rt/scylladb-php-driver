php_driver_module(exceptions ext_scylladb::exceptions)

target_sources(exceptions PRIVATE
        exceptions.c
)

php_driver_generate_arginfo(exceptions
        exceptions.stub.php
)
