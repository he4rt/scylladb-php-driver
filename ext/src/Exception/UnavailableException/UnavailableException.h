#pragma once

#include <zend_types.h>

void PhpDriverDefineUnavailableException(zend_class_entry* exceptionInterface, zend_class_entry* executionException);