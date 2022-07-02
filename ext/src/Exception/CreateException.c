#include <php.h>
#include <zend_exceptions.h>

#include "CreateException.h"

zend_object*
php_driver_exception_create_object(zend_class_entry* ce)
{
  zend_object* obj = zend_ce_exception->create_object(ce);

  return obj;
}