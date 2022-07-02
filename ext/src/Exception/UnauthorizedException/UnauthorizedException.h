#pragma once

#include <zend_types.h>

void PhpDriverDefineUnauthorizedException(zend_class_entry* exceptionInterface, zend_class_entry* validationException);