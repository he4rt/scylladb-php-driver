/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: 8dab513c933cc6d86546a9e27e7605b6fcbad4fa */

static zend_class_entry *register_class_Cassandra_TimestampGenerator_Monotonic(zend_class_entry *class_entry_Cassandra_TimestampGenerator)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\TimestampGenerator", "Monotonic", NULL);
	class_entry = zend_register_internal_class_with_flags(&ce, NULL, ZEND_ACC_FINAL|ZEND_ACC_NO_DYNAMIC_PROPERTIES);
	zend_class_implements(class_entry, 1, class_entry_Cassandra_TimestampGenerator);

	return class_entry;
}
