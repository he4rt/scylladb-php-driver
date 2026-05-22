php_driver_module(datetime ext_scylladb::datetime)

target_sources(datetime PRIVATE
        Date.cpp
        Time.cpp
        Timestamp.cpp
        Timeuuid.cpp
        DateTimeInternal.cpp
        Duration.cpp
)

php_driver_generate_arginfo(datetime
        Date.stub.php
        Timestamp.stub.php
        Duration.stub.php
        Timeuuid.stub.php
        Time.stub.php
)
