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

#include <cassandra_driver.h>

#include "FutureInterface_arginfo.h"

#include "FutureClose/FutureClose.h"
#include "FuturePreparedStatement/FuturePreparedStatement.h"
#include "FutureRows/FutureRows.h"
#include "FutureSession/FutureSession.h"
#include "FutureValue/FutureValue.h"

zend_class_entry* php_driver_future_ce = NULL;

void
php_driver_define_Future()
{
  php_driver_future_ce = register_class_Cassandra_Future();

  php_driver_define_FuturePreparedStatement(php_driver_future_ce);
  php_driver_define_FutureRows(php_driver_future_ce);
  php_driver_define_FutureSession(php_driver_future_ce);
  php_driver_define_FutureValue(php_driver_future_ce);
  php_driver_define_FutureClose(php_driver_future_ce);
}
