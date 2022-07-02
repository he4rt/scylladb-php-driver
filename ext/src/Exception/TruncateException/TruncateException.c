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

#include <php.h>

#include <CassandraDriverAPI.h>

#include "TruncateException.h"
#include "TruncateException_arginfo.h"
#include "../CreateException.h"

PHP_DRIVER_API zend_class_entry* phpDriverTruncateExceptionCe = NULL;

void
PhpDriverDefineTruncateException(zend_class_entry* exceptionInterface, zend_class_entry* serverException)
{
  phpDriverTruncateExceptionCe = register_class_Cassandra_Exceptions_TruncateException(
    serverException,
    exceptionInterface);

  phpDriverTruncateExceptionCe->create_object = php_driver_exception_create_object;
}
