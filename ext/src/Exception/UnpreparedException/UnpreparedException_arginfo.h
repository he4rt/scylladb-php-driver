/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: c3babea8e6a5c74a4c68f2f1d0b53e2279dd7f42 */




static const zend_function_entry class_Cassandra_Exceptions_UnpreparedException_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Exceptions_UnpreparedException(zend_class_entry *class_entry_Cassandra_Exceptions_ValidationException, zend_class_entry *class_entry_Cassandra_Exceptions_ExceptionInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exceptions", "UnpreparedException", class_Cassandra_Exceptions_UnpreparedException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exceptions_ValidationException);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exceptions_ExceptionInterface);

	return class_entry;
}
