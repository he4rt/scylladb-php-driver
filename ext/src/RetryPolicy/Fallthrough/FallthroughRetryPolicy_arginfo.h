/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: a4810fdc6340363205d7c8c31d179d9b616cf0db */




static const zend_function_entry class_Cassandra_RetryPolicy_FallthroughPolicy_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_RetryPolicy_FallthroughPolicy(zend_class_entry *class_entry_Cassandra_RetryPolicy_RetryPolicyInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\RetryPolicy", "FallthroughPolicy", class_Cassandra_RetryPolicy_FallthroughPolicy_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_RetryPolicy_RetryPolicyInterface);

	return class_entry;
}
