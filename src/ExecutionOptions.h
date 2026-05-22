#pragma once

#include <php.h>
#include <php_scylladb_types.h>

BEGIN_EXTERN_C()
int php_scylladb_execution_options_build_local_from_array(php_scylladb_execution_options *self, zval *options);
END_EXTERN_C()
