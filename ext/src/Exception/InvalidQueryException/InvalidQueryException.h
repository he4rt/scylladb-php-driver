#pragma once

#include <zend_types.h>

void
PhpDriverDefineInvalidQueryException(zend_class_entry* exceptionInterface, zend_class_entry* validationException);