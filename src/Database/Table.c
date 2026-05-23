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

#include "php_scylladb.h"

#include "Database/ResultDecoder.h"

#include "Table_arginfo.h"

zval
php_scylladb_table_build_options(CassIterator* iterator ) {
  const char *name;
  size_t name_length;
  zval zoptions;

  array_init(&zoptions);
  if (!iterator) {
    return zoptions;
  }
  while (cass_iterator_next(iterator)) {
    const CassValue *value = nullptr;
    if (cass_iterator_get_meta_field_name(iterator, &name, &name_length) == CASS_OK) {
      if ((name_length == sizeof("keyspace_name") - 1 &&
           memcmp(name, "keyspace_name", name_length) == 0) ||
          (name_length == sizeof("table_name") - 1 &&
           memcmp(name, "table_name", name_length) == 0) ||
          (name_length == sizeof("columnfamily_name") - 1 &&
           memcmp(name, "columnfamily_name", name_length) == 0)) {
        break;
      }
      value = cass_iterator_get_meta_field_value(iterator);
      if (value) {
        const CassDataType *data_type = cass_value_data_type(value);
        if (data_type) {
          zval zvalue;
          ZVAL_UNDEF(&zvalue);
          if (php_scylladb_value(value,
                                  data_type,
                                  &zvalue ) == SUCCESS) {
            add_assoc_zval_ex(&zoptions, name, name_length, &zvalue);
          }
        }
      }
    }
  }

  return zoptions;
}
