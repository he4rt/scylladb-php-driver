/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 11dcd847063dd683db33aa545fcfd472b136132f */




static const zend_function_entry class_Cassandra_TimestampGenerators_ServerSideGenerator_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_TimestampGenerators_ServerSideGenerator(zend_class_entry *class_entry_Cassandra_TimestampGenerators_TimestampGeneratorInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\TimestampGenerators", "ServerSideGenerator", class_Cassandra_TimestampGenerators_ServerSideGenerator_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_TimestampGenerators_TimestampGeneratorInterface);

	return class_entry;
}
