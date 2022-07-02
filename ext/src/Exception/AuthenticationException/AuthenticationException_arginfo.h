/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 334647ae263602f4e6d518817812573148d1a61e */




static const zend_function_entry class_Cassandra_Exceptions_AuthenticationException_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Exceptions_AuthenticationException(zend_class_entry *class_entry_Exception, zend_class_entry *class_entry_Cassandra_Exceptions_ExceptionInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exceptions", "AuthenticationException", class_Cassandra_Exceptions_AuthenticationException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Exception);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exceptions_ExceptionInterface);

	return class_entry;
}
