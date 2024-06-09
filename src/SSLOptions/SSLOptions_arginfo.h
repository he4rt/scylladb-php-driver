/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 6982d1ad7e035f58f6a5a01adcfcced658559a84 */




static const zend_function_entry class_Cassandra_SSLOptions_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_SSLOptions(void)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra", "SSLOptions", class_Cassandra_SSLOptions_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;

	return class_entry;
}
