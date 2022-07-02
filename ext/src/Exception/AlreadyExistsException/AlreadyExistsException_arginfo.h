/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: bcfbb188c42fa0249852d5a958c2d82ce7af6e24 */




static const zend_function_entry class_Cassandra_Exceptions_AlreadyExistsException_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Exceptions_AlreadyExistsException(zend_class_entry *class_entry_Exception, zend_class_entry *class_entry_Cassandra_Exceptions_ExceptionInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exceptions", "AlreadyExistsException", class_Cassandra_Exceptions_AlreadyExistsException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Exception);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exceptions_ExceptionInterface);

	return class_entry;
}
