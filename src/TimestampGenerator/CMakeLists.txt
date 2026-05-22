php_scylladb_module(timestamp_generator ext_scylladb::timestamp_generator)

target_sources(timestamp_generator PRIVATE
        Monotonic.cpp
        ServerSide.cpp
)

php_scylladb_generate_arginfo(timestamp_generator
        Monotonic.stub.php
        ServerSide.stub.php
        TimestampGenerator.stub.php
)

php_scylladb_generate_descriptor(timestamp_generator
        Monotonic.stub.php
        ServerSide.stub.php
        TimestampGenerator.stub.php
)
