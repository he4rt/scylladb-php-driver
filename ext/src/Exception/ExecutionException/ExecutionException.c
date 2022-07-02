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
#include <zend_exceptions.h>

#include <CassandraDriverAPI.h>

#include "../CreateException.h"
#include "ExecutionException.h"
#include "ExecutionException_arginfo.h"

PHP_DRIVER_API zend_class_entry* phpDriverExecutionExceptionCe = NULL;

void
PhpDriverDefineExecutionException(zend_class_entry* exceptionInterface)
{
  phpDriverExecutionExceptionCe = register_class_Cassandra_Exceptions_ExecutionException(
    zend_ce_exception,
    exceptionInterface);

  phpDriverExecutionExceptionCe->create_object = php_driver_exception_create_object;
}
