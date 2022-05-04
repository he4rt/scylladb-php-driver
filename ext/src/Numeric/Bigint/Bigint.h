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

#ifndef PHP_DRIVER_BIGINT_H
#define PHP_DRIVER_BIGINT_H

#include <Numeric/Bigint.h>
#include <php.h>

void php_driver_define_Bigint(zend_class_entry* value_interface, zend_class_entry* numeric_interface);

#endif /* PHP_DRIVER_BIGINT_H */
