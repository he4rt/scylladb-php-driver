/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: e68e2e13d282070d13aa5d1fea7f4f59c27e75da */




static const zend_function_entry class_Cassandra_Exceptions_ReadTimeoutException_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Exceptions_ReadTimeoutException(zend_class_entry *class_entry_Cassandra_Exceptions_TimeoutException, zend_class_entry *class_entry_Cassandra_Exceptions_ExceptionInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exceptions", "ReadTimeoutException", class_Cassandra_Exceptions_ReadTimeoutException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exceptions_TimeoutException);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exceptions_ExceptionInterface);

	return class_entry;
}
