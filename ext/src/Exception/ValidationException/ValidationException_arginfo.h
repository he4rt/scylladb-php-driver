/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: fbde6c18d60423d6e0e14f55df81252508d5cedb */




static const zend_function_entry class_Cassandra_Exceptions_ValidationException_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Exceptions_ValidationException(zend_class_entry *class_entry_Exception, zend_class_entry *class_entry_Cassandra_Exceptions_ExceptionInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exceptions", "ValidationException", class_Cassandra_Exceptions_ValidationException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Exception);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exceptions_ExceptionInterface);

	return class_entry;
}
