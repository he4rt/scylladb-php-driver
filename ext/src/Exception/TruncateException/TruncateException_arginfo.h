/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 27e39021b8dd42de847f0725f8c76b8b12fd1c42 */




static const zend_function_entry class_Cassandra_Exceptions_TruncateException_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Exceptions_TruncateException(zend_class_entry *class_entry_Cassandra_Exceptions_ServerException, zend_class_entry *class_entry_Cassandra_Exceptions_ExceptionInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exceptions", "TruncateException", class_Cassandra_Exceptions_TruncateException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exceptions_ServerException);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exceptions_ExceptionInterface);

	return class_entry;
}
