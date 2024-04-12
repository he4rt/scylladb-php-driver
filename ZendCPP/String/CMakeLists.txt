add_library(zend_strings STATIC
        Builder.cpp
        Builder.h
)
add_library(Zend::String ALIAS zend_strings)
scylladb_php_library(zend_strings OFF ${PHP_SCYLLADB_OPTIMISE_FOR_CURRENT_MACHINE} ON)