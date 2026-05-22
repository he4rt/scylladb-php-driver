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

#include <Zend/zend_smart_str.h>
#include <cassandra.h>
#include <php_driver.h>
#include <php_driver_types.h>


zval php_driver_type_from_data_type(const CassDataType* data_type);

int php_driver_type_validate(zval* object, const char* object_name);
int php_driver_type_compare(php_driver_type* type1, php_driver_type* type2);
void php_driver_type_string(php_driver_type* type, smart_str* smart);

zval php_driver_type_scalar(CassValueType type);
const char* php_driver_scalar_type_name(CassValueType type);

zval php_driver_type_set(zval* value_type);
zval php_driver_type_set_from_value_type(CassValueType type);

zval php_driver_type_collection(zval* value_type);
zval php_driver_type_collection_from_value_type(CassValueType type);

zval php_driver_type_map(zval* key_type, zval* value_type);
zval php_driver_type_map_from_value_types(CassValueType key_type,
                                                  CassValueType value_type);

zval php_driver_type_tuple();

zval php_driver_type_user_type();

zval php_driver_type_custom(const char* name, size_t name_length);

int php_driver_parse_column_type(const char* validator, size_t validator_len,
                                 int* reversed_out, int* frozen_out,
                                 zval* type_out);

void php_driver_scalar_init(INTERNAL_FUNCTION_PARAMETERS);
