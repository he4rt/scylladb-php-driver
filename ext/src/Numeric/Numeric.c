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

#include "Numeric_arginfo.h"

#include "Bigint/Bigint.h"
#include "Decimal/Decimal.h"
#include "Float/Float.h"
#include "Smallint/Smallint.h"
#include "Tinyint/Tinyint.h"
#include "Varint/Varint.h"

zend_class_entry* php_driver_numeric_ce = NULL;

void
php_driver_define_Numeric(zend_class_entry* value_interface)
{
  php_driver_numeric_ce = register_class_Cassandra_Numeric();

  php_driver_define_Bigint(value_interface, php_driver_bigint_ce);
  php_driver_define_Decimal(value_interface, php_driver_bigint_ce);
  php_driver_define_Float(value_interface, php_driver_bigint_ce);
  php_driver_define_Smallint(value_interface, php_driver_bigint_ce);
  php_driver_define_Tinyint(value_interface, php_driver_bigint_ce);
  php_driver_define_Varint(value_interface, php_driver_bigint_ce);
}
