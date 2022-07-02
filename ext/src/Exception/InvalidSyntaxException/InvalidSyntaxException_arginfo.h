/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 7b2e0456e79f65530961b4b0f98f0494bc544e44 */




static const zend_function_entry class_Cassandra_Exceptions_InvalidSyntaxException_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Exceptions_InvalidSyntaxException(zend_class_entry *class_entry_Cassandra_Exceptions_ValidationException, zend_class_entry *class_entry_Cassandra_Exceptions_ExceptionInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exceptions", "InvalidSyntaxException", class_Cassandra_Exceptions_InvalidSyntaxException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exceptions_ValidationException);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exceptions_ExceptionInterface);

	return class_entry;
}
