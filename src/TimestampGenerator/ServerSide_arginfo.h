/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 2cca1faf77a92db67185db2895b3dff58adeb358 */




static const zend_function_entry class_Cassandra_TimestampGenerator_ServerSide_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_TimestampGenerator_ServerSide(zend_class_entry *class_entry_Cassandra_TimestampGenerator)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\TimestampGenerator", "ServerSide", class_Cassandra_TimestampGenerator_ServerSide_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_TimestampGenerator);

	return class_entry;
}
