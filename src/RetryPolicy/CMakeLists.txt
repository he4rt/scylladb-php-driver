php_driver_module(retry_policy ext_scylladb::retry_policy)

target_sources(retry_policy PRIVATE
        DefaultPolicy.cpp
        DowngradingConsistency.cpp
        Logging.cpp
        Fallthrough.cpp
        RetryPolicy.cpp
)

php_driver_generate_arginfo(retry_policy
        DefaultPolicy.stub.php
        DowngradingConsistency.stub.php
        Fallthrough.stub.php
        Logging.stub.php
        RetryPolicy.stub.php
)
