#pragma once

#include <CassandraDriverAPI.h>
#include <zend_types.h>

extern PHP_DRIVER_API zend_class_entry* phpDriverExceptionInterfaceCe;

extern PHP_DRIVER_API zend_class_entry* phpDriverAlreadyExistsExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverAuthenticationExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverConfigurationExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverDivideByZeroExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverDomainExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverExecutionExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverInvalidQueryExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverInvalidSyntaxExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverIsBootstrappingExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverOverloadedExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverProtocolExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverServerExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverTimeoutExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverReadTimeoutExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverWriteTimeoutExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverTruncateExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverUnauthorizedExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverUnavailableExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverUnpreparedExceptionCe;
extern PHP_DRIVER_API zend_class_entry* phpDriverValidationExceptionCe;

void PhpDriverDefineExceptions();