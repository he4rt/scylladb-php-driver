add_library(zend_strings STATIC Builder.cpp)
add_library(Zend::String ALIAS zend_strings)

scylladb_php_library(zend_strings "${ENABLE_SANITIZERS}" "${CPU_TYPE}" "${ENABLE_LTO}")
