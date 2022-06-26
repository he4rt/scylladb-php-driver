#include <zend_types.h>

#ifndef LIBPHPCASSANDRA_EXT_SRC_RETRYPOLICY_LOGGING_LOGGING_H_
#define LIBPHPCASSANDRA_EXT_SRC_RETRYPOLICY_LOGGING_LOGGING_H_

extern zend_class_entry* phpDriverRetryPolicyLoggingCe;

void PhpDriverDefineRetryPolicyLogging(zend_class_entry* retryPolicyInterfaceCe);

#endif // LIBPHPCASSANDRA_EXT_SRC_RETRYPOLICY_LOGGING_LOGGING_H_
