#pragma once

#include <CassandraDriver.h>
#include <php.h>

#include "FutureClose.h"
#include "FuturePreparedStatement.h"
#include "FutureRows.h"
#include "FutureSession.h"
#include "FutureValue.h"

extern PHP_DRIVER_API zend_class_entry* php_driver_future_ce;
