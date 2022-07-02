/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 964b097fbe2e682b479c2f54be030391ad306f81 */




static const zend_function_entry class_Cassandra_Exceptions_ServerException_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Exceptions_ServerException(zend_class_entry *class_entry_Exception, zend_class_entry *class_entry_Cassandra_Exceptions_ExceptionInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exceptions", "ServerException", class_Cassandra_Exceptions_ServerException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Exception);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exceptions_ExceptionInterface);

	return class_entry;
}
