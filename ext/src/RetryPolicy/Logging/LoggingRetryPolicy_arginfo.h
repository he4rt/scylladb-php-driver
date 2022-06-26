/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: f5b7c4cb2585a8c4e4496bfdbadac78979fba174 */

ZEND_BEGIN_ARG_INFO_EX(arginfo_class_Cassandra_RetryPolicy_LoggingRetryPolicy___construct, 0, 0, 1)
	ZEND_ARG_OBJ_INFO(0, retryPolicy, Cassandra\\RetryPolicy\\RetryPolicyInterface, 0)
ZEND_END_ARG_INFO()


ZEND_METHOD(Cassandra_RetryPolicy_LoggingRetryPolicy, __construct);


static const zend_function_entry class_Cassandra_RetryPolicy_LoggingRetryPolicy_methods[] = {
	ZEND_ME(Cassandra_RetryPolicy_LoggingRetryPolicy, __construct, arginfo_class_Cassandra_RetryPolicy_LoggingRetryPolicy___construct, ZEND_ACC_PUBLIC)
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_RetryPolicy_LoggingRetryPolicy(zend_class_entry *class_entry_Cassandra_RetryPolicy_RetryPolicyInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\RetryPolicy", "LoggingRetryPolicy", class_Cassandra_RetryPolicy_LoggingRetryPolicy_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_RetryPolicy_RetryPolicyInterface);

	return class_entry;
}
