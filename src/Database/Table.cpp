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

#include "php_driver.h"

#include "util/result.h"
BEGIN_EXTERN_C()
#include "Table_arginfo.h"
zend_class_entry *php_driver_table_ce = NULL;

zval
php_driver_table_build_options(CassIterator* iterator ) {
  const char *name;
  size_t name_length;
  zval zoptions;


  array_init(&zoptions);
  while (cass_iterator_next(iterator)) {
    const CassValue *value = NULL;
    if (cass_iterator_get_meta_field_name(iterator, &name, &name_length) == CASS_OK) {
      if (strncmp(name, "keyspace_name", name_length) == 0 ||
          strncmp(name, "table_name", name_length) == 0 ||
          strncmp(name, "columnfamily_name", name_length) == 0) {
        break;
      }
      value = cass_iterator_get_meta_field_value(iterator);
      if (value) {
        const CassDataType *data_type = cass_value_data_type(value);
        if (data_type) {
          zval zvalue;
          ZVAL_UNDEF(&zvalue);
          if (php_driver_value(value,
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

void php_driver_define_Table()
{
  php_driver_table_ce = register_class_Cassandra_Table();
}
END_EXTERN_C()