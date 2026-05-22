/**
 * Copyright 2015-2017 DataStax, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#pragma once

#include <cassandra.h>
#include <php.h>
#include <php_scylladb_types.h>

int php_scylladb_validate_object(zval* object, zval* ztype);
int php_scylladb_value_type(char* type, CassValueType* value_type);

int php_scylladb_collection_from_set(php_scylladb_set* set, CassCollection** collection_ptr);
int php_scylladb_collection_from_collection(php_scylladb_collection* coll, CassCollection** collection_ptr);
int php_scylladb_collection_from_map(php_scylladb_map* map, CassCollection** collection_ptr);

int php_scylladb_tuple_from_tuple(php_scylladb_tuple* tuple, CassTuple** output);

int php_scylladb_user_type_from_user_type_value(php_scylladb_user_type_value* user_type_value, CassUserType** output);
