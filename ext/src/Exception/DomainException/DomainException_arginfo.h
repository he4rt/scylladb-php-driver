/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: a6fc2cbc375eff9c1a575b0f0dfcf903920e5b1f */




static const zend_function_entry class_Cassandra_Exceptions_DomainException_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_Exceptions_DomainException(zend_class_entry *class_entry_Exception, zend_class_entry *class_entry_Cassandra_Exceptions_ExceptionInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\Exceptions", "DomainException", class_Cassandra_Exceptions_DomainException_methods);
	class_entry = zend_register_internal_class_ex(&ce, class_entry_Exception);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_Exceptions_ExceptionInterface);

	return class_entry;
}
