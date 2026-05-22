php_scylladb_module(datetime ext_scylladb::datetime)

target_sources(datetime PRIVATE
        Date.c
        Time.c
        Timestamp.c
        Timeuuid.c
        DateTimeInternal.c
        Duration.c
)

php_scylladb_generate_arginfo(datetime
        Date.stub.php
        Timestamp.stub.php
        Duration.stub.php
        Timeuuid.stub.php
        Time.stub.php
)

php_scylladb_generate_descriptor(datetime
        Date.stub.php
        Timestamp.stub.php
        Duration.stub.php
        Timeuuid.stub.php
        Time.stub.php
)
