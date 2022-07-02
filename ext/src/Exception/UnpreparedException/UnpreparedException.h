#pragma once

#include <zend_types.h>

void PhpDriverDefineUnpreparedException(zend_class_entry* exceptionInterface, zend_class_entry* validationException);