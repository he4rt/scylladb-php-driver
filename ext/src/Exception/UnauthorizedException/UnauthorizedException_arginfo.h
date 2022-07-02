/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 026c701c13b3058c7efeb366e4ed185741d7721b */




static const zend_function_entry class_Cassandra_Exceptions_UnauthorizedException_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Exceptions_UnauthorizedException(zend_class_entry *class_entry_Cassandra_Exceptions_ValidationException, zend_class_entry *class_entry_Cassandra_Exceptions_ExceptionInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exceptions", "UnauthorizedException", class_Cassandra_Exceptions_UnauthorizedException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Cassandra_Exceptions_ValidationException);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exceptions_ExceptionInterface);

	return class_entry;
}
