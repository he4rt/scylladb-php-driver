#pragma once

#include "php_driver_types.h"
#include <cassandra.h>
#include <zend_types.h>

int php_driver_validate_object(zval* object, zval* ztype);
int php_driver_value_type(char* type, CassValueType* value_type);

int php_driver_collection_from_set(php_driver_set* set, CassCollection** collection_ptr);
int php_driver_collection_from_collection(php_driver_collection* coll, CassCollection** collection_ptr);
int php_driver_collection_from_map(php_driver_map* map, CassCollection** collection_ptr);

int php_driver_tuple_from_tuple(php_driver_tuple* tuple, CassTuple** output);

int php_driver_user_type_from_user_type_value(php_driver_user_type_value* user_type_value, CassUserType** output);
