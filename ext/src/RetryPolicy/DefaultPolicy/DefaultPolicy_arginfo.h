/* This is a generated file, edit the .stub.php file instead.
 * Stub hash: d98cb2781b6ee0102892067f49cee2ed53bc3127 */




static const zend_function_entry class_Cassandra_RetryPolicy_DefaultPolicy_methods[] = {
	ZEND_FE_END
};

static zend_class_entry *register_class_Cassandra_RetryPolicy_DefaultPolicy(zend_class_entry *class_entry_Cassandra_RetryPolicy_RetryPolicyInterface)
{
	zend_class_entry ce, *class_entry;

	INIT_NS_CLASS_ENTRY(ce, "Cassandra\\RetryPolicy", "DefaultPolicy", class_Cassandra_RetryPolicy_DefaultPolicy_methods);
	class_entry = zend_register_internal_class_ex(&ce, NULL);
	class_entry->ce_flags |= ZEND_ACC_FINAL;
	zend_class_implements(class_entry, 1, class_entry_Cassandra_RetryPolicy_RetryPolicyInterface);

	return class_entry;
}
